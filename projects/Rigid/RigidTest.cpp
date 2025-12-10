#include <memory>
#include <vector>

// zeno basics
#include <zeno/ListObject.h>
#include <zeno/DictObject.h>
#include <zeno/NumericObject.h>
#include <zeno/PrimitiveObject.h>
#include <zeno/types/IGeometryObject.h>
#include <zeno/logger.h>
#include <zeno/utils/UserData.h>
#include <zeno/zeno.h>
#include <zeno/utils/fileio.h>

#include "RigidTest.h"

// convex decomposition
#include <VHACD/inc/VHACD.h>
//#include <hacdHACD.h>
//#include <hacdICHull.h>
//#include <hacdVector.h>

// bullet basics
#include <BulletCollision/CollisionDispatch/btCollisionDispatcherMt.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMt.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h>
#include <LinearMath/btConvexHullComputer.h>
#include <btBulletDynamicsCommon.h>

// multibody dynamcis
#include <BulletDynamics/Featherstone/btMultiBody.h>
#include <BulletDynamics/Featherstone/btMultiBodyJointFeedback.h>


// multibody inverse kinematics/dynamics
#include "BussIK/IKTrajectoryHelper.h"
#include <Bullet3Common/b3HashMap.h>
#include "BulletInverseDynamics/MultiBodyTree.hpp"
#include "BulletInverseDynamics/btMultiBodyTreeCreator.hpp"
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <zeno/io/iocommon.h>

using namespace zenoio;

static void writebtVector3(const btVector3FloatData& vec3, RAPIDJSON_WRITER& writer) {
    writer.StartArray();
    writer.Double(vec3.m_floats[0]);
    writer.Double(vec3.m_floats[1]);
    writer.Double(vec3.m_floats[2]);
    writer.Double(vec3.m_floats[3]);
    writer.EndArray();
}

static void writebtTransform(const btTransform& trans, RAPIDJSON_WRITER& writer) {
    JsonObjScope batch(writer);
    {
        btTransformData dataOut;
        trans.serialize(dataOut);

        writer.Key("m_basis");
        {
            writer.StartArray();
            btMatrix3x3FloatData basis = dataOut.m_basis;
            writebtVector3(basis.m_el[0], writer);
            writebtVector3(basis.m_el[1], writer);
            writebtVector3(basis.m_el[2], writer);
            writer.EndArray();
        }
        writer.Key("m_origin");
        btVector3FloatData origin = dataOut.m_origin;
        writebtVector3(origin, writer);
    }
}

static void writebtCollisionShape(const btCollisionShape* shape, RAPIDJSON_WRITER& writer) {
    if (!shape) {
        writer.Null();
        return;
    }

    JsonObjScope shapeObj(writer);

    // 通用部分
    writer.Key("m_shapeType");
    int type = shape->getShapeType();
    writer.Int(type);

    writer.Key("m_name");
    writer.String(shape->getName());

    // 子类分派
    switch (type) {
    case BOX_SHAPE_PROXYTYPE: {
        const btBoxShape* box = static_cast<const btBoxShape*>(shape);
        btVector3 halfExt = box->getHalfExtentsWithMargin();
        btVector3FloatData v;
        halfExt.serializeFloat(v);
        writer.Key("m_halfExtents");
        writebtVector3(v, writer);
        break;
    }
    case SPHERE_SHAPE_PROXYTYPE: {
        const btSphereShape* sphere = static_cast<const btSphereShape*>(shape);
        writer.Key("m_radius");
        writer.Double(sphere->getRadius());
        break;
    }
    case COMPOUND_SHAPE_PROXYTYPE: {
        const btCompoundShape* compound = static_cast<const btCompoundShape*>(shape);
        writer.Key("m_children");
        writer.StartArray();
        for (int i = 0; i < compound->getNumChildShapes(); ++i) {
            const btCollisionShape* child = compound->getChildShape(i);
            const btTransform& childTransform = compound->getChildTransform(i);

            writer.StartObject();
            writer.Key("m_childTransform");
            writebtTransform(childTransform, writer);
            writer.Key("m_childShape");
            writebtCollisionShape(child, writer); // ✅ 递归！
            writer.EndObject();
        }
        writer.EndArray();
        break;
    }
    default:
        writer.Key("m_info");
        writer.String("Unsupported shape type");
        break;
    }
}

static void writebtVector3(const btVector3& v, RAPIDJSON_WRITER& writer) {
    writer.StartArray();
    writer.Double(v.x());
    writer.Double(v.y());
    writer.Double(v.z());
    writer.EndArray();
}

// 序列化 btTriangleMesh
static void writebtTriangleMesh(const btTriangleMesh* mesh, RAPIDJSON_WRITER& writer) {
    if (!mesh) {
        writer.Null();
        return;
    }

    JsonObjScope obj(writer);

    // 获取基本网格信息
    const unsigned char* vertexBase = nullptr;
    const unsigned char* indexBase = nullptr;
    int numVerts = 0, numFaces = 0;
    int vertexStride = 0, indexStride = 0;
    PHY_ScalarType vertexType, indexType;

    mesh->getLockedReadOnlyVertexIndexBase(
        &vertexBase, numVerts, vertexType, vertexStride,
        &indexBase, indexStride, numFaces, indexType);

    writer.Key("m_numVertices");
    writer.Int(numVerts);
    writer.Key("m_numTriangles");
    writer.Int(numFaces);

    writer.Key("m_vertices");
    writer.StartArray();
    for (int i = 0; i < numVerts; ++i) {
        const btVector3* v = reinterpret_cast<const btVector3*>(vertexBase + i * vertexStride);
        writebtVector3(*v, writer);
    }
    writer.EndArray();

    writer.Key("m_triangles");
    writer.StartArray();
    for (int i = 0; i < numFaces; ++i) {
        const unsigned char* tri = indexBase + i * indexStride;

        writer.StartArray();
        if (indexType == PHY_INTEGER) {
            const int* idx = reinterpret_cast<const int*>(tri);
            writer.Int(idx[0]);
            writer.Int(idx[1]);
            writer.Int(idx[2]);
        }
        else if (indexType == PHY_SHORT) {
            const unsigned short* idx = reinterpret_cast<const unsigned short*>(tri);
            writer.Int(idx[0]);
            writer.Int(idx[1]);
            writer.Int(idx[2]);
        }
        else {
            writer.Int(-1);
            writer.Int(-1);
            writer.Int(-1);
        }
        writer.EndArray();
    }
    writer.EndArray();

    mesh->unLockReadOnlyVertexBase(0);
}

static void writebtRigidBody(const btRigidBody* body, RAPIDJSON_WRITER& writer) {
    if (!body) {
        writer.Null();
        return;
    }

    JsonObjScope obj(writer);

    // 质量
    writer.Key("m_mass");
    writer.Double(1.0 / body->getInvMass());

    // 世界变换
    writer.Key("m_worldTransform");
    writebtTransform(body->getWorldTransform(), writer);

    // 线速度
    btVector3FloatData linVel;
    body->getLinearVelocity().serializeFloat(linVel);
    writer.Key("m_linearVelocity");
    writebtVector3(linVel, writer);

    // 角速度
    btVector3FloatData angVel;
    body->getAngularVelocity().serializeFloat(angVel);
    writer.Key("m_angularVelocity");
    writebtVector3(angVel, writer);

    // 阻尼
    writer.Key("m_linearDamping");
    writer.Double(body->getLinearDamping());
    writer.Key("m_angularDamping");
    writer.Double(body->getAngularDamping());

    // 重力
    btVector3FloatData gravity;
    body->getGravity().serializeFloat(gravity);
    writer.Key("m_gravity");
    writebtVector3(gravity, writer);

    // 碰撞形状
    const btCollisionShape* shape = body->getCollisionShape();
    if (shape) {
        writer.Key("m_collisionShape");
        JsonObjScope shapeObj(writer);

        writer.Key("m_shapeType");
        writer.Int(shape->getShapeType());

        switch (shape->getShapeType()) {
        case BOX_SHAPE_PROXYTYPE: {
            const btBoxShape* box = static_cast<const btBoxShape*>(shape);
            btVector3 halfExt = box->getHalfExtentsWithMargin();
            btVector3FloatData v;
            halfExt.serializeFloat(v);
            writer.Key("m_halfExtents");
            writebtVector3(v, writer);
            break;
        }
        case SPHERE_SHAPE_PROXYTYPE: {
            const btSphereShape* sphere = static_cast<const btSphereShape*>(shape);
            writer.Key("m_radius");
            writer.Double(sphere->getRadius());
            break;
        }
        default:
            writer.Key("m_info");
            writer.String("Unsupported shape type");
            break;
        }
    }
}

std::string BulletTransform::serialize_json() const {
    rapidjson::StringBuffer s;
    RAPIDJSON_WRITER writer(s);
    writebtTransform(trans, writer);
    return s.GetString();
}

std::string BulletTriangleMesh::serialize_json() const {
    rapidjson::StringBuffer s;
    RAPIDJSON_WRITER writer(s);
    writebtTriangleMesh(&mesh, writer);
    return s.GetString();
}


std::string BulletObjectProxy::serialize_json() const {
    if (!m_pHost) return "";

    rapidjson::StringBuffer s;
    RAPIDJSON_WRITER writer(s);
    {
        JsonObjScope batch(writer);
        writer.Key("MotionState");
        {
            JsonObjScope _batch(writer);
            writer.Key("graphics world trans");
            writebtTransform(m_pHost->myMotionState->m_graphicsWorldTrans, writer);

            writer.Key("Mass Offset");
            writebtTransform(m_pHost->myMotionState->m_centerOfMassOffset, writer);

            writer.Key("start world trans");
            writebtTransform(m_pHost->myMotionState->m_startWorldTrans, writer);

            writer.Key("montionstate has use point");
            writer.Bool(m_pHost->myMotionState->m_userPointer ? true : false);
        }
        writer.Key("RigidBody");
        {
            writebtRigidBody(m_pHost->body.get(), writer);
        }
        writer.Key("colShape");
        {
            writebtCollisionShape(m_pHost->colShape->shape.get(), writer);
        }
    }
    std::string strJson = s.GetString();
    return strJson;
}

std::string BulletCollisionShapeProxy::serialize_json() const {
    if (!m_pHost) return "";

    rapidjson::StringBuffer s;
    RAPIDJSON_WRITER writer(s);
    writebtCollisionShape(m_pHost->shape.get(), writer);
    return s.GetString();
}



namespace {
using namespace zeno;

/*
 *  Bullet Position & Rotation
 */

struct BulletMakeTransform : zeno::INode {
    virtual void apply() override {
        auto trans = std::make_unique<BulletTransform>();
        trans->trans.setIdentity();
        if (has_input("translate")) {
            auto origin = toVec3f(get_input2_vec3f("translate"));
            trans->trans.setOrigin(zeno::vec_to_other<btVector3>(origin));
        }
        if (has_input("rotation")) {
            zeno::reflect::Any rot = get_param_result("rotation");
            if (rot.type().hash_code() == gParamType_Vec3f) {
                zeno::vec3f rotation = zeno::reflect::any_cast<zeno::vec3f>(rot);
                trans->trans.setRotation(zeno::vec_to_other<btQuaternion>(rotation)); // ypr
            }
            else if (rot.type().hash_code() == gParamType_Vec4f) {
                zeno::vec4f rotation = zeno::reflect::any_cast<zeno::vec4f>(rot);
                trans->trans.setRotation(zeno::vec_to_other<btQuaternion>(rotation));
            }
        }
        set_output("trans", std::move(trans));
    }
};

ZENDEFNODE(BulletMakeTransform, {
    {{gParamType_Vec3f, "translate"},  {gParamType_AnyNumeric, "rotation", ""}},
    {{gParamType_BulletTransform, "trans"}},
    {},
    {"Bullet"},
});

struct BulletWorldSetConsList : zeno::INode {
    virtual void apply() override {
        auto world = safe_uniqueptr_cast<BulletWorldProxy>(clone_input("world"));
        auto consListObj = safe_uniqueptr_cast<ListObject>(clone_input("consList"));
        std::vector<std::shared_ptr<BulletConstraint>> consList;
        for (int i = 0; i < consListObj->size(); i++) {
            zeno::IObject* obj = consListObj->get(i);
            if (auto pConstraint = dynamic_cast<BulletConstraintProxy*>(obj)) {
                consList.push_back(pConstraint->m_pHost);
            }
        }
        world->m_pHost->setConstraintList(std::move(consList));
        set_output("world", std::move(world));
    }
};

ZENDEFNODE(BulletWorldSetConsList, {
    {{gParamType_BulletWorld, "world"},
     {gParamType_List, "consList"}},
    {{gParamType_BulletWorld, "world"}},
    {},
    {"Bullet"},
});

struct BulletWorldSetObjList : zeno::INode {
    virtual void apply() override {
        auto world = safe_uniqueptr_cast<BulletWorldProxy>(clone_input("world"));
        auto objListObj = safe_uniqueptr_cast<ListObject>(clone_input("objList"));
        std::vector<std::shared_ptr<BulletObject>> objList;
        for (int i = 0; i < objListObj->size(); i++) {
            zany obj = objListObj->move(i);
            if (auto pObj = dynamic_cast<BulletObjectProxy*>(obj.get())) {
                objList.push_back(pObj->m_pHost);
            }
        }
        world->m_pHost->setObjectList(std::move(objList));
        set_output("world", std::move(world));
    }
};

ZENDEFNODE(BulletWorldSetObjList, {
    {{gParamType_BulletWorld, "world"}, {gParamType_List, "objList"}},
    {{gParamType_BulletWorld, "world"}},
    {},
    {"Bullet"},
});

struct BulletStepWorld : zeno::INode {
    virtual void apply() override {
        auto world = safe_uniqueptr_cast<BulletWorldProxy>(clone_input("world"));
        auto dt = get_input2_float("dt");
        auto steps = get_input2_int("steps");
        world->m_pHost->step(dt, steps);
        set_output("world", std::move(world));
    }
};

ZENDEFNODE(BulletStepWorld, {
    {{gParamType_BulletWorld, "world"}, {gParamType_Float, "dt", "0.04"}, {gParamType_Int, "steps", "1"}},
    {{gParamType_BulletWorld, "world"}},
    {},
    {"Bullet"},
});


struct BulletObjectSetDamping : zeno::INode {
    virtual void apply() override {
        auto object = safe_uniqueptr_cast<BulletObjectProxy>(clone_input("object"));
        auto dampLin = get_input2_float("dampLin");
        auto dampAug = get_input2_float("dampAug");
        log_debug("set object {} with dampLin={}, dampAug={}", (void*)object.get(), dampLin, dampAug);
        object->m_pHost->body->setDamping(dampLin, dampAug);
        set_output("object", std::move(object));
    }
};

ZENDEFNODE(BulletObjectSetDamping, {
    {{gParamType_BulletObject, "object"}, {gParamType_Float, "dampLin", "0"}, {gParamType_Float, "dampAug", "0"}},
    {{gParamType_BulletObject, "object"}},
    {},
    {"Bullet"},
});


struct BulletObjectSetFriction : zeno::INode {
    virtual void apply() override {
        auto object = safe_uniqueptr_cast<BulletObjectProxy>(clone_input("object"));
        auto friction = get_input2_float("friction");
        log_debug("set object {} with friction={}", (void*)object.get(), friction);
        object->m_pHost->body->setFriction(friction);
        set_output("object", std::move(object));
    }
};

ZENDEFNODE(BulletObjectSetFriction, {
    {{gParamType_BulletObject, "object"}, {gParamType_Float, "friction", "0"}},
    {{gParamType_BulletObject, "object"}},
    {},
    {"Bullet"},
    });


struct BulletWorldSetGravity : zeno::INode {
    virtual void apply() override {
        auto world = safe_uniqueptr_cast<BulletWorldProxy>(clone_input("world"));
        auto gravity = toVec3f(get_input2_vec3f("gravity"));
        world->m_pHost->dynamicsWorld->setGravity(zeno::vec_to_other<btVector3>(gravity));
        set_output("world", std::move(world));
    }
};

ZENDEFNODE(BulletWorldSetGravity, {
    {{gParamType_BulletWorld, "world"}, {gParamType_Vec3f, "gravity", "0,0,-9.8"}},
    {{gParamType_BulletWorld, "world"}},
    {},
    {"Bullet"},
});



struct BulletExtractTransform : zeno::INode {
    virtual void apply() override {
        auto trans = &dynamic_cast<BulletTransform*>(get_input("trans"))->trans;
        set_output_vec3f("origin", toAbiVec3f(vec3f(other_to_vec<3>(trans->getOrigin()))));
        set_output_vec4f("rotation", toAbiVec4f(vec4f(other_to_vec<4>(trans->getRotation()))));
    }
};

ZENDEFNODE(BulletExtractTransform, {
    {{gParamType_BulletTransform, "trans"}},
    {{gParamType_Vec3f,"origin"}, {gParamType_Vec4f, "rotation"}},
    {},
    {"Bullet"},
});


/*
 * Bullet Object
 */
struct BulletMakeObject : zeno::INode {
    virtual void apply() override {
        auto shape = safe_uniqueptr_cast<BulletCollisionShapeProxy>(clone_input("shape"));
        auto mass = get_input2_float("mass");
        auto trans = safe_uniqueptr_cast<BulletTransform>(clone_input("trans"));
        auto object = std::make_shared<BulletObject>(
            mass, trans->trans, shape->m_pHost);
        object->body->setDamping(0, 0);
        object->body->forceActivationState(4);
        auto proxy = std::make_unique<BulletObjectProxy>();
        proxy->m_pHost = object;
        set_output("object", std::move(proxy));
    }
};

ZENDEFNODE(BulletMakeObject, {
    {{gParamType_BulletCollisionShape, "shape"}, {gParamType_BulletTransform, "trans"}, {gParamType_Float, "mass", "0"}},
    {{gParamType_BulletObject, "object"}},
    {},
    {"Bullet"},
    });


struct BulletMakeWorld : zeno::INode {
    virtual void apply() override {
        auto world = std::make_unique<BulletWorldProxy>();
        world->m_pHost = std::make_shared<BulletWorld>();
        set_output("world", std::move(world));
    }
};

ZENDEFNODE(BulletMakeWorld, {
    {},
    {{gParamType_BulletWorld, "world"}},
    {},
    {"Bullet"},
});


struct BulletObjectGetTransform : zeno::INode {
    virtual void apply() override {
        auto obj = safe_uniqueptr_cast<BulletObjectProxy>(clone_input("object"));
        auto body = obj->m_pHost->body.get();
        auto trans = std::make_unique<BulletTransform>();

        if (body && body->getMotionState()) {
            body->getMotionState()->getWorldTransform(trans->trans);
        } else {
            trans->trans = static_cast<btCollisionObject*>(body)->getWorldTransform();
        }
        set_output("trans", std::move(trans));
    }
};

ZENDEFNODE(BulletObjectGetTransform, {
    {{gParamType_BulletObject, "object"}},
    {{gParamType_BulletTransform, "trans"}},
    {},
    {"Bullet"},
    });


struct BulletObjectApplyForce :zeno::INode {
    virtual void apply() override {
        auto object = safe_uniqueptr_cast<BulletObjectProxy>(clone_input("object"));
        auto forceImpulse = toVec3f(get_input2_vec3f("ForceImpulse"));
        auto torqueImpulse = toVec3f(get_input2_vec3f("TorqueImpulse"));
        object->m_pHost->body->applyCentralImpulse(zeno::vec_to_other<btVector3>(forceImpulse));
        object->m_pHost->body->applyTorqueImpulse(zeno::vec_to_other<btVector3>(torqueImpulse));
        set_output("object", std::move(object));
    }
};

ZENDEFNODE(BulletObjectApplyForce, {
    {
        {gParamType_BulletObject, "object"},
        {gParamType_Vec3f, "ForceImpulse", "0,0,0"},
        {gParamType_Vec3f, "TorqueImpulse", "0,0,0"}
    },
    {{gParamType_BulletObject, "object"}},
    {},
    {"Bullet"},
});


struct BulletObjectSetRestitution : zeno::INode {
    virtual void apply() override {
        auto object = safe_uniqueptr_cast<BulletObjectProxy>(clone_input("object"));
        auto restitution = get_input2_float("restitution");
        log_debug("set object {} with restituion={}", (void*)object.get(), restitution);
        object->m_pHost->body->setRestitution(restitution);
        set_output("object", std::move(object));
    }
};

ZENDEFNODE(BulletObjectSetRestitution, {
    {{gParamType_BulletObject, "object"}, {gParamType_Float, "restitution", "0"}},
    {{gParamType_BulletObject, "object"}},
    {},
    {"Bullet"},
});

/*
 * Bullet Geometry
 */
struct PrimitiveToBulletMesh : zeno::INode {
    virtual void apply() override {
        auto prim = get_input_Geometry("prim")->toPrimitiveObject();
        auto mesh = std::make_unique<BulletTriangleMesh>();
        auto pos = prim->attr<zeno::vec3f>("pos");
        for (int i = 0; i < prim->tris.size(); i++) {
            auto f = prim->tris[i];
            mesh->mesh.addTriangle(
                zeno::vec_to_other<btVector3>(pos[f[0]]),
                zeno::vec_to_other<btVector3>(pos[f[1]]),
                zeno::vec_to_other<btVector3>(pos[f[2]]), true);
        }
        set_output("mesh", std::move(mesh));
    }
};

ZENDEFNODE(PrimitiveToBulletMesh, {
    {{gParamType_Geometry, "prim"}},
    {{gParamType_BulletTriangleMesh, "mesh"}},
    {},
    {"Bullet"},
    });


/*
 *  Bullet Collision
 */

struct BulletMakeBoxShape : zeno::INode {
    virtual void apply() override {
        auto size = toVec3f(get_input2_vec3f("semiSize"));
        auto proxy = std::make_unique<BulletCollisionShapeProxy>();
        proxy->m_pHost = std::make_shared<BulletCollisionShape>(
            std::make_unique<btBoxShape>(zeno::vec_to_other<btVector3>(size)));
        set_output("shape", std::move(proxy));
    }
};

ZENDEFNODE(BulletMakeBoxShape, {
    {{gParamType_Vec3f, "semiSize", "1,1,1"}},
    {{gParamType_BulletCollisionShape, "shape"}},
    {},
    {"Bullet"},
    });

struct BulletMakeSphereShape : zeno::INode {
    virtual void apply() override {
        auto radius = get_input2_float("radius");
        auto shape = std::make_unique<BulletCollisionShapeProxy>();
        shape->m_pHost = std::make_shared<BulletCollisionShape>(
            std::make_unique<btSphereShape>(btScalar(radius)));
        set_output("shape", std::move(shape));
    }
};

ZENDEFNODE(BulletMakeSphereShape, {
    {{gParamType_Float, "radius", "1"}},
    {{gParamType_BulletCollisionShape, "shape"}},
    {},
    {"Bullet"},
    });


// it moves mesh to CollisionShape
struct BulletMakeConvexHullShape : zeno::INode {
    virtual void apply() override {
#if 1
        auto meshObj = safe_uniqueptr_cast<BulletTriangleMesh>(clone_input("triMesh"));
        auto triMesh = &meshObj->mesh;
        auto inShape = std::make_unique<btConvexTriangleMeshShape>(triMesh);
        auto hull = std::make_unique<btShapeHull>(inShape.get());
        auto margin = get_input2_float("margin");
        hull->buildHull(margin, 0);
        auto convex = std::make_unique<btConvexHullShape>(
            (const btScalar*)hull->getVertexPointer(), hull->numVertices());
        convex->setMargin(btScalar(margin));
#else
        auto prim = get_input_Geometry("prim")->toPrimitiveObject();
        auto convexHC = std::make_unique<btConvexHullComputer>();
        std::vector<float> vertices;
        vertices.reserve(prim->size() * 3);
        for (int i = 0; i < prim->size(); i++) {
            btVector3 coor = vec_to_other<btVector3>(prim->verts[i]);
            vertices.push_back(coor[0]);
            vertices.push_back(coor[1]);
            vertices.push_back(coor[2]);
        }
        auto margin = get_input2_float("margin");
        convexHC->compute(vertices.data(), sizeof(float) * 3, vertices.size() / 3, 0.0f, 0.0f);
        auto convex = std::make_unique<btConvexHullShape>(
            &(convexHC->vertices[0].getX()), convexHC->vertices.size());
        convex->setMargin(btScalar(margin));
#endif

        // auto convex = std::make_unique<btConvexPointCloudShape>();
        // btVector3* points = new btVector3[inShape->getNumVertices()];
        // for(int i=0;i<inShape->getNumVertices(); i++)
        // {
        //     btVector3 v;
        //     inShape->getVertex(i, v);
        //     points[i]=v;
        // }
        auto shape = std::make_unique<BulletCollisionShapeProxy>();
        shape->m_pHost = std::make_shared<BulletCollisionShape>(std::move(convex));
        set_output("shape", std::move(shape));
    }
};

ZENDEFNODE(BulletMakeConvexHullShape, {
    {{gParamType_BulletTriangleMesh, "triMesh"}, {gParamType_Float, "margin", "0"}},
    {{gParamType_BulletCollisionShape, "shape"}},
    {},
    {"Bullet"},
    });


struct BulletConstraintSetBreakThres : zeno::INode {
    virtual void apply() override {
        auto cons = safe_uniqueptr_cast<BulletConstraintProxy>(clone_input("constraint"));
        cons->m_pHost->setBreakingThreshold(get_input2_float("threshold"));
        set_output("constraint", std::move(cons));
    }
};

ZENDEFNODE(BulletConstraintSetBreakThres, {
    {{gParamType_BulletConstraint, "constraint"}, {gParamType_Float, "threshold", "3.0"}},
    {{gParamType_BulletConstraint, "constraint"}},
    {},
    {"Bullet"},
    });


struct BulletMakeConstraint : zeno::INode {
    virtual void apply() override {
        auto constraintType = zsString2Std(get_input2_string("constraintType"));
        auto obj1 = safe_uniqueptr_cast<BulletObjectProxy>(clone_input("obj1"));
        auto iter = get_input2_int("iternum");
        std::shared_ptr<BulletConstraint> cons;
        if (has_input("obj2")) {
            auto obj2 = safe_uniqueptr_cast<BulletObjectProxy>(clone_input("obj2"));
            cons = std::make_shared<BulletConstraint>(obj1->m_pHost->body.get(), obj2->m_pHost->body.get(), constraintType);
        } else {
            cons = std::make_shared<BulletConstraint>(obj1->m_pHost->body.get(), constraintType);
        }
        cons->constraint->setOverrideNumSolverIterations(iter);

        auto constraint = std::make_unique<BulletConstraintProxy>();
        constraint->m_pHost = cons;
        set_output("constraint", std::move(constraint));
    }
};

ZENDEFNODE(BulletMakeConstraint, {
    {{gParamType_BulletObject, "obj1"}, {gParamType_BulletObject, "obj2"}, {gParamType_Int, "iternum", "100"}},
    {{gParamType_BulletConstraint, "constraint"}},
    {{"enum ConeTwist Fixed Gear Generic6Dof Generic6DofSpring Generic6DofSpring2 Hinge Hinge2 Point2Point Slider Universal", "constraintType", "Fixed"}},
    {"Bullet"},
    });


#if 0

struct BulletTransformSetBasisEuler : zeno::INode {
    virtual void apply() override {
        auto trans = safe_uniqueptr_cast<BulletTransform>(move_input("trans"))->trans;
        auto euler = toVec3f(get_input2_vec3f("eulerZYX"));
        trans.getBasis().setEulerZYX(euler[0], euler[1], euler[2]);
    }
};

ZENDEFNODE(BulletTransformSetBasisEuler, {
    {{gParamType_BulletTransform, "trans"}, {gParamType_Vec3f, "eulerZYX"}},
    {},
    {},
    {"Bullet"}
});

struct BulletMakeFrameFromPivotAxis : zeno::INode {
	virtual void apply() override {
		auto pivot = zeno::vec_to_other<btVector3>(toVec3f(get_input2_vec3f("pivot")));
		auto axis = zeno::vec_to_other<btVector3>(toVec3f(get_input2_vec3f("axis")));

        auto trans = std::make_unique<BulletTransform>();

        trans->trans.setOrigin(pivot);
        trans->trans.getBasis().setValue(axis.getX(),axis.getX(),axis.getX(),axis.getY(),axis.getY(),axis.getY(),axis.getZ(),axis.getZ(),axis.getZ());

		set_output("frame", std::move(trans));
	}
};

ZENDEFNODE(BulletMakeFrameFromPivotAxis, {
	{{gParamType_Vec3f, "pivot"}, {gParamType_Vec3f, "axis"}},
	{{gParamType_BulletTransform, "frame"}},
	{},
	{"Bullet"}
});

struct BulletQuatRotate : zeno::INode {
    virtual void apply() override {
        auto quat = zeno::vec_to_other<btQuaternion>(toVec4f(get_input2_vec4f("quat")));
        auto v = zeno::vec_to_other<btVector3>(toVec3f(get_input2_vec3f("vec3")));

        auto res = quatRotate(quat, v);
        set_output_vec3f("vec3", toAbiVec3f(vec3f(other_to_vec<3>(res))));
    }
};

ZENDEFNODE(BulletQuatRotate, {
    {{gParamType_Vec4f, "quat"}, {gParamType_Vec3f, "vec3"}},
    {{gParamType_Vec3f,"vec3"}},
    {},
    {"Bullet"}
});

struct BulletComposeTransform : zeno::INode {
    virtual void apply() override {
        auto transFirst = dynamic_cast<BulletTransform*>(get_input("transFirst"))->trans;
        auto transSecond = dynamic_cast<BulletTransform*>(get_input("transSecond"))->trans;
        auto trans = std::make_unique<BulletTransform>();
        trans->trans = transFirst * transSecond;
        set_output("trans", std::move(trans));
    }
};

ZENDEFNODE(BulletComposeTransform, {
    {{gParamType_BulletTransform, "transFirst"}, {gParamType_BulletTransform, "transSecond"}},
    {{gParamType_BulletTransform, "trans"}},
    {},
    {"Bullet"},
});





struct VHACDParameters
{
    bool m_run;
    VHACD::IVHACD::Parameters m_paramsVHACD;
    VHACDParameters(void)
    {
        m_run = true;
    }
};

struct PrimitiveConvexDecompositionV : zeno::INode {
    /*
    *  Use VHACD to do convex decomposition
    */
    // V-HACD (Volumetric Hierarchical Approximate Convex Decomposition): 这是一种基于体积的凸分解算法，它会生成一个凸体的近似表示，这些凸体的数量和质量可以由用户控制。
    // 该算法优先处理形状的内部，而不是边缘，从而在保留形状主要特性的同时，还能控制结果凸体的数量。这是一种生成高质量凸体集的有效方法。
    virtual void apply() override {
        auto prim = get_input_Geometry("prim")->toPrimitiveObject();
        auto &pos = prim->attr<zeno::vec3f>("pos");

        //auto resolution = get_input2_int("resolution");


        std::vector<float> points;
        std::vector<int> triangles;

        for (size_t i = 0; i < pos.size(); i++){
            points.push_back(pos[i][0]);
            points.push_back(pos[i][1]);
            points.push_back(pos[i][2]);
        }

        for (size_t i = 0; i < prim->tris.size(); i++){
            triangles.push_back(prim->tris[i][0]);
            triangles.push_back(prim->tris[i][1]);
            triangles.push_back(prim->tris[i][2]);
        }

        VHACDParameters params;
        // TODO: get more parameters from INode, currently it is only for testing.
        params.m_paramsVHACD.m_resolution = 100000; // Maximum number of voxels generated during the voxelization stage (default=100,000, range=10,000-16,000,000)
        params.m_paramsVHACD.m_depth = 20; // Maximum number of clipping stages. During each split stage, parts with a concavity higher than the user defined threshold are clipped according the "best" clipping plane (default=20, range=1-32)
        params.m_paramsVHACD.m_concavity = 0.001; // Maximum allowed concavity (default=0.0025, range=0.0-1.0)
        params.m_paramsVHACD.m_planeDownsampling = 4; // Controls the granularity of the search for the "best" clipping plane (default=4, range=1-16)
        params.m_paramsVHACD.m_convexhullDownsampling  = 4; // Controls the precision of the convex-hull generation process during the clipping plane selection stage (default=4, range=1-16)
        params.m_paramsVHACD.m_alpha = 0.05; // Controls the bias toward clipping along symmetry planes (default=0.05, range=0.0-1.0)
        params.m_paramsVHACD.m_beta = 0.05; // Controls the bias toward clipping along revolution axes (default=0.05, range=0.0-1.0)
        params.m_paramsVHACD.m_gamma = 0.0005; // Controls the maximum allowed concavity during the merge stage (default=0.00125, range=0.0-1.0)
        params.m_paramsVHACD.m_pca = 0; // Enable/disable normalizing the mesh before applying the convex decomposition (default=0, range={0,1})
        params.m_paramsVHACD.m_mode = 0; // 0: voxel-based approximate convex decomposition, 1: tetrahedron-based approximate convex decomposition (default=0, range={0,1})
        params.m_paramsVHACD.m_maxNumVerticesPerCH = 64; // Controls the maximum number of triangles per convex-hull (default=64, range=4-1024)
        params.m_paramsVHACD.m_minVolumePerCH = 0.0001; // Controls the adaptive sampling of the generated convex-hulls (default=0.0001, range=0.0-0.01)
        params.m_paramsVHACD.m_convexhullApproximation = true; // Enable/disable approximation when computing convex-hulls (default=1, range={0,1})
        params.m_paramsVHACD.m_oclAcceleration = true; // Enable/disable OpenCL acceleration (default=0, range={0,1})


        VHACD::IVHACD* interfaceVHACD = VHACD::CreateVHACD();
        bool res = interfaceVHACD->Compute(&points[0], 3, (unsigned int)points.size() / 3,
                                           &triangles[0], 3, (unsigned int)triangles.size() / 3, params.m_paramsVHACD);


        // save output
        create_ListObject();
        auto listPrim = create_ListObject();
        listPrim->clear();


        unsigned int nConvexHulls = interfaceVHACD->GetNConvexHulls();
        //std::cout<< "Generate output:" << nConvexHulls << " convex-hulls" << std::endl;
        printf("Generate output: %d convex-hulls \n", nConvexHulls);

        bool good_ch_flag = true;
        VHACD::IVHACD::ConvexHull ch;
        size_t vertexOffset = 0; // triangle index start from 1
        for (size_t c = 0; c < nConvexHulls; c++) {
            interfaceVHACD->GetConvexHull(c, ch);
            size_t nPoints = ch.m_nPoints;
            size_t nTriangles = ch.m_nTriangles;

            auto outprim = std::make_shared<zeno::PrimitiveObject>();
            outprim->resize(nPoints);
            outprim->tris.resize(nTriangles);

            auto &outpos = outprim->add_attr<zeno::vec3f>("pos");

            if (nPoints > 0) {
                for (size_t i = 0; i < nPoints; i ++) {
                    size_t ind = i * 3;
                    outpos[i] = zeno::vec3f(ch.m_points[ind], ch.m_points[ind + 1], ch.m_points[ind + 2]);
                }
            }
            else{
                good_ch_flag = false;
            }
            if (nTriangles > 0)
            {
                for (size_t i = 0; i < nTriangles; i++) {
                    size_t ind = i * 3;
                    outprim->tris[i] = zeno::vec3i(ch.m_triangles[ind], ch.m_triangles[ind + 1],ch.m_triangles[ind + 2]);
                }
            }
            else{
                good_ch_flag = false;
            }

            if(good_ch_flag) {
                listPrim->push_back(std::move(outprim));
            }
        }

        interfaceVHACD->Clean();
        interfaceVHACD->Release();

        set_output("listPrim", std::move(listPrim));
    }
};

ZENDEFNODE(PrimitiveConvexDecompositionV, {
    {{gParamType_Geometry, "prim"}},
    {{gParamType_List, "listPrim"}},
    {},
    {"Bullet"},
});

struct PrimitiveConvexDecomposition : zeno::INode {
    virtual void apply() override {
        auto prim = get_input_Geometry("prim")->toPrimitiveObject();
        auto &pos = prim->attr<zeno::vec3f>("pos");

        std::vector<HACD::Vec3<HACD::Real>> points;
        std::vector<HACD::Vec3<long>> triangles;

        for (int i = 0; i < pos.size(); i++) {
            points.push_back(
                    zeno::vec_to_other<HACD::Vec3<HACD::Real>>(pos[i]));
        }

        for (int i = 0; i < prim->tris.size(); i++) {
            triangles.push_back(
                    zeno::vec_to_other<HACD::Vec3<long>>(prim->tris[i]));
        }

        // HACD (Hierarchical Approximate Convex Decomposition): 这是一种使用图理论来进行凸分解的方法。它将形状的几何图形视为无向图，然后使用图割来进行凸分解。
        // 这种方法能够生成比较精细和准确的凸分解，但是相比于V-HACD，它可能会生成更多的凸体。

        HACD::HACD hacd;
        hacd.SetPoints(points.data());
        hacd.SetNPoints(points.size());
        hacd.SetTriangles(triangles.data());
        hacd.SetNTriangles(triangles.size());

        
        // 用于设置Hierarchical Approximate Convex Decomposition（HACD）算法中紧凑性的权重因子（w）。
        // 紧凑性权重（Compacity Weight）是一个控制生成的凸体形状的参数。更具体地说，紧凑性权重决定了在生成凸体时，紧凑性（形状的体积和表面积之比）与其他因素（如生成的凸体数量等）的相对重要性。
        // 如果设置一个较高的紧凑性权重，HACD将优先生成紧凑的凸体，这可能会增加生成的凸体数量。相反，如果设置一个较低的紧凑性权重，HACD可能会生成较少但形状较为扁平的凸体。
        auto CompacityWeight = get_input2_float("CompacityWeight");
        hacd.SetCompacityWeight(CompacityWeight);


        auto VolumeWeight = get_input2_float("VolumeWeight");
        hacd.SetVolumeWeight(VolumeWeight);

        // 这个参数的具体含义是：HACD将尽可能地生成接近设定数量的凸体簇。例如，如果你设置了hacd.SetNClusters(10)，那么HACD将尽量生成接近10个的凸体簇。
        // 需要注意的是，HACD可能无法生成精确数量的凸体簇，因为实际生成的数量取决于输入形状的复杂性和其他参数。此外，如果生成的凸体簇数量超过了设定的值，HACD可能会通过合并一些凸体簇来降低总数。
        auto NClusters = get_input2_int("NClusters");
        hacd.SetNClusters(NClusters);

        // hacd.SetNVerticesPerCH(n): 这个函数设定了每个生成的凸体（Convex Hulls）最多应包含的顶点数。该值设定的越高，凸体形状的精度就越高，但也会导致计算复杂性增加。
        auto NVerticesPerCH = get_input2_int("NVerticesPerCH");
        hacd.SetNVerticesPerCH(NVerticesPerCH);

        // hacd.SetConcavity(c): 这个函数设置了一个阈值，用于确定凸体生成的凹度。该值设定的越高，允许生成的凸体的凹度就越大，这可能导致生成的凸体数量减少，但凸体形状可能变得更加复杂。
        auto Concavity = get_input2_float("Concavity");
        hacd.SetConcavity(100.0);

        // hacd.SetAddExtraDistPoints(b): 这个函数决定是否在凸体分解中添加额外的距离点。设置为true可以增加生成的凸体的准确度，但也会增加计算复杂性。
        auto AddExtraDistPoints = get_input2_bool("AddExtraDistPoints");
        hacd.SetAddExtraDistPoints(AddExtraDistPoints);

        // hacd.SetAddNeighboursDistPoints(b): 这个函数决定是否在凸体分解中添加邻近的距离点。设置为true可以增加生成的凸体的准确度，但也会增加计算复杂性。
        auto AddNeighboursDistPoints = get_input2_bool("AddNeighboursDistPoints");
        hacd.SetAddNeighboursDistPoints(AddNeighboursDistPoints);

        // hacd.SetAddFacesPoints(b): 这个函数决定是否在凸体分解中添加面的点。设置为true可以增加生成的凸体的准确度，但也会增加计算复杂性。
        auto AddFacesPoints = get_input2_bool("AddFacesPoints");
        hacd.SetAddFacesPoints(AddFacesPoints);

        hacd.Compute();
        size_t nClusters = hacd.GetNClusters();

        auto listPrim = create_ListObject();
        listPrim->clear();

        printf("hacd got %d clusters\n", nClusters);
        for (size_t c = 0; c < nClusters; c++) {
            size_t nPoints = hacd.GetNPointsCH(c);
            size_t nTriangles = hacd.GetNTrianglesCH(c);
            printf("hacd cluster %d have %d points, %d triangles\n",
                       c, nPoints, nTriangles);

            points.clear();
            points.resize(nPoints);
            triangles.clear();
            triangles.resize(nTriangles);
            hacd.GetCH(c, points.data(), triangles.data());

            auto outprim = std::make_shared<zeno::PrimitiveObject>();
            outprim->resize(nPoints);
            outprim->tris.resize(nTriangles);

            auto &outpos = outprim->add_attr<zeno::vec3f>("pos");
            for (size_t i = 0; i < nPoints; i++) {
                auto p = points[i];
                //printf("point %d: %f %f %f\n", i, p.X(), p.Y(), p.Z());
                outpos[i] = zeno::vec3f(p.X(), p.Y(), p.Z());
            }

            for (size_t i = 0; i < nTriangles; i++) {
                auto p = triangles[i];
                //printf("triangle %d: %d %d %d\n", i, p.X(), p.Y(), p.Z());
                outprim->tris[i] = zeno::vec3i(p.X(), p.Y(), p.Z());
            }

            listPrim->push_back(create_GeometryObject(outprim.get()));
        }

        set_output("listPrim", std::move(listPrim));
    }
};

ZENDEFNODE(PrimitiveConvexDecomposition, {
    {
        {gParamType_Geometry, "prim"},
        {gParamType_Float,"CompacityWeight","0.1"},
        {gParamType_Float,"VolumeWeight","0.0"},
        {gParamType_Int,"NClusters","2"},
        {gParamType_Int,"NVerticesPerCH","100"},
        {gParamType_Float,"Concavity","100.0"},
        {gParamType_Bool,"AddExtraDistPoints","false"},
        {gParamType_Bool,"AddNeighboursDistPoints","false"},
        {gParamType_Bool,"AddFacesPoints","false"}

        },
    {{gParamType_List, "listPrim"}},
    {},
    {"Bullet"},
});


struct BulletMakeStaticPlaneShape : zeno::INode {
	virtual void apply() override {
		auto planeNormal = zeno::vec_to_other<btVector3>(toVec3f(get_input2_vec3f("planeNormal")));
		auto planeConstant = btScalar(get_input2_float("planeConstant"));

		auto shape = std::make_unique<BulletCollisionShape>(std::make_unique<btStaticPlaneShape>(planeNormal, planeConstant));
		set_output("shape", std::move(shape));
	}
};

ZENDEFNODE(BulletMakeStaticPlaneShape, {
    {{gParamType_Vec3f, "planeNormal"}, {gParamType_Float, "planeConstant", "40"}},
	{{gParamType_BulletCollisionShape, "shape"}},
	{},
	{"Bullet"},
});

struct BulletMakeCapsuleShape : zeno::INode {
	virtual void apply() override {
		auto radius = get_input2_float("radius");
		auto height = get_input2_float("height");

		auto shape = std::make_unique<BulletCollisionShape>(std::make_unique<btCapsuleShape>(btScalar(radius), btScalar(height)));
		set_output("shape", std::move(shape));
	}
};

ZENDEFNODE(BulletMakeCapsuleShape, {
	{{gParamType_Float, "radius", "1"}, {gParamType_Float, "height", "1"}},
	{{gParamType_BulletCollisionShape, "shape"}},
	{},
	{"Bullet"},
});

struct BulletMakeCylinderShape : zeno::INode {
	virtual void apply() override {
		auto halfExtents = zeno::vec_to_other<btVector3>(toVec3f(get_input2_vec3f("halfExtents")));

		auto shape = std::make_unique<BulletCollisionShape>(std::make_unique<btCylinderShape>(halfExtents));
		set_output("shape", std::move(shape));
	}
};

ZENDEFNODE(BulletMakeCylinderShape, {
    {{gParamType_Vec3f, "halfExtents"}},
	{{gParamType_BulletCollisionShape, "shape"}},
	{},
	{"Bullet"},
});

struct BulletMakeCompoundShape : zeno::INode {
    virtual void apply() override {
        auto compound = std::make_unique<btCompoundShape>();
        auto shape = std::make_unique<BulletCompoundShape>(std::move(compound));
        set_output("compound", std::move(shape));
    }
};

ZENDEFNODE(BulletMakeCompoundShape, {
    {},
    {{gParamType_BulletCollisionShape, "compound"}},
    {},
    {"Bullet"},
});

struct BulletCompoundAddChild : zeno::INode {
    virtual void apply() override {
        auto compound = safe_uniqueptr_cast<BulletCompoundShape>(move_input("compound"));
        auto childShape = safe_uniqueptr_cast<BulletCollisionShapeProxy>(clone_input("childShape"));
        auto trans = safe_uniqueptr_cast<BulletTransform>(move_input("trans"))->trans;

        compound->addChild(trans, std::move(childShape));
        set_output("compound", get_input("compound"));
    }
};

ZENDEFNODE(BulletCompoundAddChild, {
    {{gParamType_BulletCollisionShape, "compound"},
     {gParamType_BulletCollisionShape, "childShape"},
     {gParamType_BulletTransform, "trans"}},
    {{gParamType_BulletCollisionShape, "compound"}},
    {},
    {"Bullet"},
});

struct BulletCompoundFinalize : zeno::INode {
    virtual void apply() override {
        auto compound = safe_uniqueptr_cast<BulletCompoundShape>(move_input("compound_shape"));
        auto cpdShape = static_cast<btCompoundShape*>(compound->shape.get());

        if (compound->children.size() != cpdShape->getNumChildShapes()) {
            printf("ridiculous. sth is wrong.\n");
            throw std::runtime_error("?????");
        }
        auto nchs = compound->children.size();
        std::vector<float> masses(nchs, 1.f);
        btTransform principalTrans;
        btVector3 inertia;
        cpdShape->calculatePrincipalAxisTransform(masses.data(), principalTrans, inertia);
        for (int rbi = 0; rbi != nchs; ++rbi) {
            cpdShape->updateChildTransform(rbi, principalTrans.inverse() * cpdShape->getChildTransform(rbi));
        }

        set_output("compound_shape", get_input("compound_shape"));
    }
};

ZENDEFNODE(BulletCompoundFinalize, {
    {{gParamType_BulletCollisionShape, "compound_shape"}},
    {{gParamType_BulletCollisionShape, "compound_shape"}},
    {},
    {"Bullet"},
});

struct BulletMakeGlueCompoundShape : zeno::INode {
	virtual void apply() override {
		auto compound = std::make_unique<BulletGlueCompoundShape>();
		set_output("compound", std::move(compound));
	}
};

ZENDEFNODE(BulletMakeGlueCompoundShape, {
	{
	},
	{
        {gParamType_BulletCollisionShape, "shape"},
	},
	{},
	{"Bullet"},
});

/*
struct BulletMakeGlueObjectList : zeno::INode {
    std::shared_ptr<ListObject> objectList = create_ListObject();

    virtual void apply() override {
        auto shape = safe_uniqueptr_cast<BulletGlueCompoundShape>(move_input("glueCompShape"));
        auto mass = get_input2_float("mass");
        auto trans = safe_uniqueptr_cast<BulletTransform>(move_input("trans"));
        objectList->arr.clear();
        for (auto const &comp: shape->comps) {
            auto object = std::make_unique<BulletObject>(
                mass, trans->trans, comp);
            object->body->setDamping(0, 0);
            objectList->push_back(std::move(object));
        }
        log_debug("glueobjeclist length={}", objectList->size());
        set_output("objectList", std::move(objectList));
    }
};

ZENDEFNODE(BulletMakeGlueObjectList, {
    {"glueCompShape", "trans", {gParamType_Float, "mass", "0"}},
    {"objectList"},
    {},
    {"Bullet"},
});
       */

struct BulletGlueCompoundAddChild : zeno::INode {
    virtual void apply() override {
        auto compound = safe_uniqueptr_cast<BulletGlueCompoundShape>(move_input("compound"));
        auto childShape = safe_uniqueptr_cast<BulletCollisionShapeProxy>(clone_input("childShape"));
        auto trans = safe_uniqueptr_cast<BulletTransform>(move_input("trans"))->trans;
        auto mass = get_input2_float("mass");

        compound->addChild(mass, std::move(trans), std::move(childShape));
        set_output("compound", get_input("compound"));
    }
};

ZENDEFNODE(BulletGlueCompoundAddChild, {
	{
		{gParamType_BulletCollisionShape, "compound"},
		{gParamType_BulletCollisionShape, "childShape"},
		{gParamType_BulletTransform, "trans"},
        {gParamType_Float, "mass"},
	},
	{
		{gParamType_BulletCollisionShape, "compound"},
	},
	{},
	{"Bullet"},
});

struct BulletGlueCompoundUpdateGlueList : zeno::INode {
    virtual void apply() override {
        auto compound = safe_uniqueptr_cast<BulletGlueCompoundShape>(move_input("compound"));
        auto glueList = get_input_ListObject("glueListVec2i")->getLiterial<vec2i>();
	compound->clearGlues();
    for (int i = 0; i < glueList.size(); i++) {
        auto [x, y] = glueList[i];
        compound->addGlue(x, y);
    }
    compound->solveGlueToComps();
        set_output("compound", get_input("compound"));
    }
};

ZENDEFNODE(BulletGlueCompoundUpdateGlueList, {
	{
		{gParamType_Unknown, "compound"},
        {gParamType_Vec2i, "glueListVec2i"},
	},
	{
		{gParamType_Unknown, "compound"},
	},
	{},
	{"Bullet"},
});

struct BulletColShapeCalcLocalInertia : zeno::INode {
	virtual void apply() override {
		auto isCompound = (zsString2Std(get_input2_string("isCompound")) == "true");
		auto mass = get_input2_float("mass");
		auto localInertia = zeno::IObject::make<zeno::NumericObject>();
		btVector3 lInertia;
		if (isCompound){
			auto colObject = safe_uniqueptr_cast<BulletCompoundShape>(move_input("colObject"));
			colObject->shape->calculateLocalInertia(btScalar(mass), lInertia);
		}
		else {
			auto colObject = safe_uniqueptr_cast<BulletCollisionShapeProxy>(clone_input("colObject"));
			colObject->shape->calculateLocalInertia(btScalar(mass), lInertia);
		}

		localInertia->set<zeno::vec3f>(zeno::vec3f(lInertia[0], lInertia[1], lInertia[2]));
		set_output("localInertia", std::move(localInertia));
	}
};

ZENDEFNODE(BulletColShapeCalcLocalInertia, {
	{{gParamType_Unknown, "colObject"}, {gParamType_Float, "mass", "1"}},
    {{gParamType_Vec3f,"localInertia"}},
	{{"enum true false", "isCompound", "false"}},
	{"Bullet"}
});

// it moves mesh to CollisionShape
struct BulletMakeConvexMeshShape : zeno::INode {
	virtual void apply() override {
		auto triMesh = &safe_uniqueptr_cast<BulletTriangleMesh>(move_input("triMesh"))->mesh;
		auto inShape = std::make_unique<btConvexTriangleMeshShape>(triMesh);

		auto shape = std::make_unique<BulletCollisionShape>(std::move(inShape));
		set_output("shape", std::move(shape));
	}
};

ZENDEFNODE(BulletMakeConvexMeshShape, {
	{{gParamType_Unknown, "triMesh"}},
	{{gParamType_Unknown, "shape"}},
	{},
	{"Bullet"},
});

struct BulletMakeBvhTriangleMeshShape : zeno::INode {
	virtual void apply() override {
		auto triMesh = &safe_uniqueptr_cast<BulletTriangleMesh>(move_input("triMesh"))->mesh;
		auto inShape = std::make_unique<btBvhTriangleMeshShape>(triMesh, true, true);

		auto shape = std::make_unique<BulletCollisionShape>(std::move(inShape));
		set_output("shape", std::move(shape));
	}
};

ZENDEFNODE(BulletMakeBvhTriangleMeshShape, {
	{{gParamType_Unknown, "triMesh"}},
	{{gParamType_Unknown, "shape"}},
	{},
	{"Bullet"},
});



struct BulletObjectForceActivation : zeno::INode {
    virtual void apply() override {
        std::map<std::string, int> ActvSta{{"ACTIVE_TAG", 1},         {"ISLAND_SLEEPING", 2},
                                           {"WANTS_DEACTIVATION", 3}, {"DISABLE_DEACTIVATION", 4},
                                           {"DISABLE_SIMULATION", 5}, {"FIXED_BASE_MULTI_BODY", 6}};

        auto object = safe_uniqueptr_cast<BulletObjectProxy>(clone_input("object"));
        auto stateTag = zsString2Std(get_input2_string("ActivationState"));

        int state = ActvSta[stateTag];
        object->body->forceActivationState(state);
        set_output("object", std::move(object));
    }
};

ZENDEFNODE(BulletObjectForceActivation, {
                                            {{gParamType_Unknown, "object"},
                                             {"enum ACTIVE_TAG ISLAND_SLEEPING WANTS_DEACTIVATION DISABLE_DEACTIVATION DISABLE_SIMULATION FIXED_BASE_MULTI_BODY",
                                              "ActivationState", "ACTIVE_TAG"}},
                                            {{gParamType_Unknown, "object"}},
                                            {},
                                            {"Bullet"},
                                        });


struct BulletInverseTransform : zeno::INode {
	virtual void apply() override {
		auto trans = safe_uniqueptr_cast<BulletTransform>(move_input("trans"));
		btTransform t = trans->trans.inverse();
		trans->trans.setOrigin(t.getOrigin());
		trans->trans.setRotation(t.getRotation());
		set_output("trans_inv", std::move(trans));
	}
};

ZENDEFNODE(BulletInverseTransform, {
	{{gParamType_Unknown, "trans"}},
	{{gParamType_Unknown, "trans_inv"}},
	{},
	{"Bullet"}
});

struct BulletObjectGetVel : zeno::INode {
    virtual void apply() override {
        auto obj = safe_uniqueptr_cast<BulletObjectProxy>(clone_input("object"));
        auto body = obj->body.get();
        auto linearVel = zeno::IObject::make<zeno::NumericObject>();
        auto angularVel = zeno::IObject::make<zeno::NumericObject>();
        linearVel->set<zeno::vec3f>(zeno::vec3f(0));
        angularVel->set<zeno::vec3f>(zeno::vec3f(0));

        if (body && body->getLinearVelocity() ) {
            auto v = body->getLinearVelocity();
            linearVel->set<zeno::vec3f>(zeno::vec3f(v.x(), v.y(), v.z()));
        }
        if (body && body->getAngularVelocity() ){
            auto w = body->getAngularVelocity();
            angularVel->set<zeno::vec3f>(zeno::vec3f(w.x(), w.y(), w.z()));
        }
        set_output("linearVel", linearVel);
        set_output("angularVel", angularVel);
    }
};

ZENDEFNODE(BulletObjectGetVel, {
    {{gParamType_Unknown, "object"}},
    {
        {gParamType_Vec3f,"linearVel"},
        {gParamType_Vec3f,"angularVel"}
    },
    {},
    {"Bullet"},
});

//struct RigidVelToPrimitive : zeno::INode {
//    virtual void apply() override {
//        auto prim = get_input_Geometry("prim")->toPrimitiveObject();
//        auto com = toVec3f(get_input2_vec3f("centroid"));
//        auto lin = toVec3f(get_input2_vec3f("linearVel"));
//        auto ang = toVec3f(get_input2_vec3f("angularVel"));
//
//        auto &pos = prim->attr<zeno::vec3f>("pos");
//        auto &vel = prim->add_attr<zeno::vec3f>("vel");
//        #pragma omp parallel for
//        for (size_t i = 0; i < prim->size(); i++) {
//            vel[i] = lin + zeno::cross(ang, pos[i] - com);
//        }
//
//        set_output("prim", get_input("prim"));
//    }
//};
//
//ZENDEFNODE(RigidVelToPrimitive, {
//    {gParamType_Geometry, "centroid", "linearVel", "angularVel"},
//    {gParamType_Geometry, "prim"},
//    {},
//    {"Bullet"},
//});


/*static class btTaskSchedulerManager {
	btAlignedObjectArray<btITaskScheduler*> m_taskSchedulers;
	btAlignedObjectArray<btITaskScheduler*> m_allocatedTaskSchedulers;

public:
	btTaskSchedulerManager() {}
	void init()
	{
		addTaskScheduler(btGetSequentialTaskScheduler());
#if BT_THREADSAFE
		if (btITaskScheduler* ts = btCreateDefaultTaskScheduler())
		{
			m_allocatedTaskSchedulers.push_back(ts);
			addTaskScheduler(ts);
		}
		addTaskScheduler(btGetOpenMPTaskScheduler());
		addTaskScheduler(btGetTBBTaskScheduler());
		addTaskScheduler(btGetPPLTaskScheduler());
		if (getNumTaskSchedulers() > 1)
		{
			// prefer a non-sequential scheduler if available
			btSetTaskScheduler(m_taskSchedulers[1]);
		}
		else
		{
			btSetTaskScheduler(m_taskSchedulers[0]);
		}
#endif  // #if BT_THREADSAFE
	}
	void shutdown()
	{
		for (int i = 0; i < m_allocatedTaskSchedulers.size(); ++i)
		{
			delete m_allocatedTaskSchedulers[i];
		}
		m_allocatedTaskSchedulers.clear();
	}

	void addTaskScheduler(btITaskScheduler* ts)
	{
		if (ts)
		{
#if BT_THREADSAFE
			// if initial number of threads is 0 or 1,
			if (ts->getNumThreads() <= 1)
			{
				// for OpenMP, TBB, PPL set num threads to number of logical cores
				ts->setNumThreads(ts->getMaxNumThreads());
			}
#endif  // #if BT_THREADSAFE
			m_taskSchedulers.push_back(ts);
		}
	}
	int getNumTaskSchedulers() const { return m_taskSchedulers.size(); }
	btITaskScheduler* getTaskScheduler(int i) { return m_taskSchedulers[i]; }
} gTaskSchedulerMgr; */


struct BulletConstraintDisplay: zeno::INode{
    virtual void apply() override {
        auto prim = get_input_Geometry("prim")->toPrimitiveObject();
        auto nlist = get_input_ListObject("nlist")->getLiterial<zeno::vec2i>();
        for(int i=0; i<nlist.size(); i++){
            auto const& n = nlist[i];
            prim->lines.push_back(zeno::vec2i(n[0], n[1]));
        }
        set_output("prim", std::move(prim));
    }
};

ZENDEFNODE(BulletConstraintDisplay, {
    {{gParamType_Geometry, "nlist"}},
    {{gParamType_Geometry, "prim"}},
    {},
    {"Bullet"},
});


struct BulletConstraintSetFrames : zeno::INode {
    virtual void apply() override {
        auto cons = safe_uniqueptr_cast<BulletConstraint>(move_input("constraint"));
	    auto frame1 = safe_uniqueptr_cast<BulletTransform>(move_input("frame1"));
        auto frame2 = safe_uniqueptr_cast<BulletTransform>(move_input("frame2"));
        auto constraintType = zsString2Std(get_input2_string("constraintType"));

        if (constraintType == "ConeTwist") {
            dynamic_cast<btConeTwistConstraint *>(cons->constraint.get())->setFrames(frame1->trans, frame2->trans);
        }
        else if (constraintType == "Fixed") {
            dynamic_cast<btFixedConstraint *>(cons->constraint.get())->setFrames(frame1->trans, frame2->trans);
        }
        else if (constraintType == "Generic6Dof") {
            dynamic_cast<btGeneric6DofConstraint *>(cons->constraint.get())->setFrames(frame1->trans, frame2->trans);
        }
        else if (constraintType == "Generic6DofSpring") {
            dynamic_cast<btGeneric6DofSpringConstraint *>(cons->constraint.get())->setFrames(frame1->trans, frame2->trans);
        }
        else if (constraintType == "Generic6DofSpring2") {
            dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->setFrames(frame1->trans, frame2->trans);
        }
        else if (constraintType == "Hinge") {
            dynamic_cast<btHingeConstraint *>(cons->constraint.get())->setFrames(frame1->trans, frame2->trans);
        }
        else if (constraintType == "Hinge2") {
            dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->setFrames(frame1->trans, frame2->trans);
        }
        else if (constraintType == "Slider") {
            dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setFrames(frame1->trans, frame2->trans);
        }
        else if (constraintType == "Universal") {
            dynamic_cast<btUniversalConstraint *>(cons->constraint.get())->setFrames(frame1->trans, frame2->trans);
        }
        set_output("constraint", std::move(cons));
    }
};

ZENDEFNODE(BulletConstraintSetFrames, {
    {{gParamType_Unknown, "constraint"}, {gParamType_Unknown, "frame1"}, {gParamType_Unknown, "frame2"}},
    {{gParamType_Unknown, "constraint"}},
    {{"enum ConeTwist Fixed Generic6Dof Generic6DofSpring Generic6DofSpring2 Hinge Hinge2 Slider Universal", "constraintType", "Universal"}},
    {"Bullet"},
});

struct BulletConstraintGetFrames : zeno::INode {
	virtual void apply() override {
		auto cons = safe_uniqueptr_cast<BulletConstraint>(move_input("constraint"));
		auto constraintType = zsString2Std(get_input2_string("constraintType"));

		auto frame1 = std::make_unique<BulletTransform>();
		auto frame2 = std::make_unique<BulletTransform>();

		if (constraintType == "ConeTwist") {
			frame1->trans = dynamic_cast<btConeTwistConstraint *>(cons->constraint.get())->getFrameOffsetA();
			frame2->trans = dynamic_cast<btConeTwistConstraint *>(cons->constraint.get())->getFrameOffsetB();
		}
		else if (constraintType == "Fixed") {
			frame1->trans = dynamic_cast<btFixedConstraint *>(cons->constraint.get())->getFrameOffsetA();
			frame2->trans = dynamic_cast<btFixedConstraint *>(cons->constraint.get())->getFrameOffsetB();
		}
		else if (constraintType == "Generic6Dof") {
			frame1->trans = dynamic_cast<btGeneric6DofConstraint *>(cons->constraint.get())->getFrameOffsetA();
			frame2->trans = dynamic_cast<btGeneric6DofConstraint *>(cons->constraint.get())->getFrameOffsetB();
		}
		else if (constraintType == "Generic6DofSpring") {
			frame1->trans = dynamic_cast<btGeneric6DofSpringConstraint *>(cons->constraint.get())->getFrameOffsetA();
			frame2->trans = dynamic_cast<btGeneric6DofSpringConstraint *>(cons->constraint.get())->getFrameOffsetB();
		}
		else if (constraintType == "Generic6DofSpring2") {
			frame1->trans = dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->getFrameOffsetA();
			frame2->trans = dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->getFrameOffsetB();
		}
		else if (constraintType == "Hinge") {
			std::cout<< "get constraint " << (void *)cons.get() << std::endl;
			frame1->trans = dynamic_cast<btHingeConstraint *>(cons->constraint.get())->getFrameOffsetA();
			frame2->trans = dynamic_cast<btHingeConstraint *>(cons->constraint.get())->getFrameOffsetB();
			std::cout << "frame1:" << frame1->trans.getOrigin()[0] << " " <<  frame1->trans.getOrigin()[1] << " " << frame1->trans.getOrigin()[2] << std::endl;
			std::cout << "frame2:" << frame2->trans.getOrigin()[0] << " " <<  frame2->trans.getOrigin()[1] << " " << frame2->trans.getOrigin()[2] << std::endl;
		}
		else if (constraintType == "Hinge2") {
			frame1->trans = dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->getFrameOffsetA();
			frame2->trans = dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->getFrameOffsetB();
		}
		else if (constraintType == "Slider") {
			frame1->trans = dynamic_cast<btSliderConstraint *>(cons->constraint.get())->getFrameOffsetA();
			frame2->trans = dynamic_cast<btSliderConstraint *>(cons->constraint.get())->getFrameOffsetB();
		}
		else if (constraintType == "Universal") {
			frame1->trans = dynamic_cast<btUniversalConstraint *>(cons->constraint.get())->getFrameOffsetA();
			frame2->trans = dynamic_cast<btUniversalConstraint *>(cons->constraint.get())->getFrameOffsetB();
		}

		set_output("frame1", std::move(frame1));
		set_output("frame2", std::move(frame2));
	}
};

ZENDEFNODE(BulletConstraintGetFrames, {
	{{gParamType_Unknown, "constraint"}},
    {{gParamType_Unknown, "frame1"}, {gParamType_Unknown, "frame2"}},
	{{"enum ConeTwist Fixed Generic6Dof Generic6DofSpring Generic6DofSpring2 Hinge Hinge2 Slider Universal", "constraintType", "Universal"}},
	{"Bullet"}
});

struct BulletConstraintSetLimitByAxis : zeno::INode {
    virtual void apply() override {
        auto cons = safe_uniqueptr_cast<BulletConstraint>(move_input("constraint"));
        auto constraintType = zsString2Std(get_input2_string("constraintType"));
        auto axisId = zsString2Std(get_input2_string("axisId"));
        int axis;
        if (axisId == "linearX"){axis = 0;}
        else if (axisId == "linearY"){axis = 1;}
        else if (axisId == "linearZ"){axis = 2;}
        else if (axisId == "angularX"){axis = 3;}
        else if (axisId == "angularY"){axis = 4;}
        else {axis=5;} // "angularZ"

        auto low = btScalar(get_input2_float("lowLimit"));
        auto high = btScalar(get_input2_float("highLimit"));

		if (has_input("axisId")){
			if (constraintType == "ConeTwist") {
				if ((high > low) && (high - low) < 1e-5) {
					dynamic_cast<btConeTwistConstraint *>(cons->constraint.get())->setLimit(axis, low); // axis >= 3
				}
			}
			else if (constraintType == "Fixed") {
				dynamic_cast<btFixedConstraint *>(cons->constraint.get())->setLimit(axis, low, high);
			}
			else if (constraintType == "Generic6Dof") {
				dynamic_cast<btGeneric6DofConstraint *>(cons->constraint.get())->setLimit(axis, low, high);
			}
			else if (constraintType == "Generic6DofSpring") {
				dynamic_cast<btGeneric6DofSpringConstraint *>(cons->constraint.get())->setLimit(axis, low, high);
			}
			else if (constraintType == "Generic6DofSpring2") {
				dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->setLimit(axis, low, high);
			}
			else if (constraintType == "Hinge2") {
				dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->setLimit(axis, low, high);
			}
			else if (constraintType == "Slider") {
				if (axis < 3) {
					dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setLowerLinLimit(low);
					dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setUpperLinLimit(high);
				}
				else{
					dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setLowerAngLimit(low);
					dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setUpperAngLimit(high);
				}

			}
			else if (constraintType == "Universal") {
				dynamic_cast<btUniversalConstraint *>(cons->constraint.get())->setLimit(axis, low, high);
			}
		}
		else {
			if (constraintType == "Hinge") {
				dynamic_cast<btHingeConstraint *>(cons->constraint.get())->setLimit(low, high);
			}
		}

        set_output("constraint", std::move(cons));
    }
};

ZENDEFNODE(BulletConstraintSetLimitByAxis, {
    {{gParamType_Unknown, "constraint"}, {gParamType_Float, "lowLimit"}, {gParamType_Float, "highLimit"}},
    {{gParamType_Unknown, "constraint"}},
    {{"enum ConeTwist Fixed Generic6Dof Generic6DofSpring Generic6DofSpring2 Hinge Hinge2 Slider Universal", "constraintType", "Universal"}, {"enum linearX linearY linearZ angularX angularY angularZ", "axisId", "linearX"}},
    {"Bullet"},
});

struct BulletConstraintSetRefFrameA : zeno::INode {
    virtual void apply() override {
        auto cons = safe_uniqueptr_cast<BulletConstraint>(move_input("constraint"));
        auto constraintType = zsString2Std(get_input2_string("constraintType"));
        auto useLinearReferenceFrameA = zsString2Std(get_input2_string("useReferenceFrameA"));

		bool flag = (useLinearReferenceFrameA == "true");

		// std::cout << "check useLinearReferenceFrameA bool: " << flag << std::endl;

        if (constraintType == "Generic6Dof") {
			dynamic_cast<btGeneric6DofConstraint *>(cons->constraint.get())->setUseLinearReferenceFrameA(flag);
        }
        else if (constraintType == "Generic6DofSpring") {
            dynamic_cast<btGeneric6DofSpringConstraint *>(cons->constraint.get())->setUseLinearReferenceFrameA(flag);
        }
        else if (constraintType == "Hinge") {
            dynamic_cast<btHingeConstraint *>(cons->constraint.get())->setUseReferenceFrameA(flag); // NAME DIFFERENCE
        }
        else if (constraintType == "Universal") {
            dynamic_cast<btUniversalConstraint *>(cons->constraint.get())->setUseLinearReferenceFrameA(flag);
        }
        set_output("constraint", std::move(cons));
    }
};

ZENDEFNODE(BulletConstraintSetRefFrameA, {
    {{gParamType_Unknown, "constraint"}},
    {{gParamType_Unknown, "constraint"}},
    {{"enum Generic6Dof Generic6DofSpring Hinge Universal", "constraintType", "Universal"}, {"enum true false", "useReferenceFrameA", "true"}},
    {"Bullet"},
});

struct BulletConstraintSetAxis : zeno::INode {
    virtual void apply() override {
        auto cons = safe_uniqueptr_cast<BulletConstraint>(move_input("constraint"));
        auto axis1 = zeno::vec_to_other<btVector3>(toVec3f(get_input2_vec3f("axis1")));
        auto axis2 = zeno::vec_to_other<btVector3>(toVec3f(get_input2_vec3f("axis2")));
        auto constraintType = zsString2Std(get_input2_string("constraintType"));


        if (constraintType == "Fixed") {
            dynamic_cast<btFixedConstraint *>(cons->constraint.get())->setAxis(axis1, axis2);
        }
        else if (constraintType == "Gear") {
            dynamic_cast<btGearConstraint *>(cons->constraint.get())->setAxisA(axis1);
            dynamic_cast<btGearConstraint *>(cons->constraint.get())->setAxisB(axis2);
        }
        else if (constraintType == "Generic6Dof") {
            dynamic_cast<btGeneric6DofConstraint *>(cons->constraint.get())->setAxis(axis1, axis2);
        }
        else if (constraintType == "Generic6DofSpring") {
            dynamic_cast<btGeneric6DofSpringConstraint *>(cons->constraint.get())->setAxis(axis1, axis2);
        }
        else if (constraintType == "Generic6DofSpring2") {
            dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->setAxis(axis1, axis2);
        }
        else if (constraintType == "Hinge") {
            dynamic_cast<btHingeConstraint *>(cons->constraint.get())->setAxis(axis1);
        }
        else if (constraintType == "Hinge2") {
            dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->setAxis(axis1, axis2);
        }
        else if (constraintType == "Point2Point") {
            dynamic_cast<btPoint2PointConstraint *>(cons->constraint.get())->setPivotA(axis1);
            dynamic_cast<btPoint2PointConstraint *>(cons->constraint.get())->setPivotB(axis2);
        }
        else if (constraintType == "Universal") {
            dynamic_cast<btUniversalConstraint *>(cons->constraint.get())->setAxis(axis1, axis2);
        }
        set_output("constraint", std::move(cons));
    }
};

ZENDEFNODE(BulletConstraintSetAxis, {
    {{gParamType_Unknown, "constraint"}, {gParamType_Float, "axis1"}, {gParamType_Float, "axis2"}},
    {{gParamType_Unknown, "constraint"}},
    {{"enum Fixed Gear Generic6Dof Generic6DofSpring Generic6DofSpring2 Hinge Hinge2 Point2Point Universal", "constraintType", "Universal"}},
    {"Bullet"},
});

struct BulletConstraintSetSpring : zeno::INode {
    virtual void apply() override {
        auto cons = safe_uniqueptr_cast<BulletConstraint>(move_input("constraint"));
        auto constraintType = zsString2Std(get_input2_string("constraintType"));
        auto axisId = zsString2Std(get_input2_string("axisId"));
        int axis;
        if (axisId == "linearX"){axis = 0;}
        else if (axisId == "linearY"){axis = 1;}
        else if (axisId == "linearZ"){axis = 2;}
        else if (axisId == "angularX"){axis = 3;}
        else if (axisId == "angularY"){axis = 4;}
        else {axis=5;} // "angularZ"
        auto enabled = get_input2_bool("enable");
        auto stiffness = btScalar(get_input2_float("stiffness"));
        auto damping = btScalar(get_input2_float("damping"));
        auto epVal = btScalar(get_input2_float("equilibriumPointVal"));

        if (constraintType == "ConeTwist") {
            dynamic_cast<btConeTwistConstraint *>(cons->constraint.get())->setDamping(damping);

        }
        if (constraintType == "Fixed") {
            dynamic_cast<btFixedConstraint *>(cons->constraint.get())->enableSpring(axis, enabled);
            dynamic_cast<btFixedConstraint *>(cons->constraint.get())->setStiffness(axis, stiffness);
            dynamic_cast<btFixedConstraint *>(cons->constraint.get())->setDamping(axis, damping);
            dynamic_cast<btFixedConstraint *>(cons->constraint.get())->setEquilibriumPoint(axis, epVal);
        }
        else if (constraintType == "Generic6DofSpring") {
            dynamic_cast<btGeneric6DofSpringConstraint *>(cons->constraint.get())->enableSpring(axis, enabled);
            dynamic_cast<btGeneric6DofSpringConstraint *>(cons->constraint.get())->setStiffness(axis, stiffness);
            dynamic_cast<btGeneric6DofSpringConstraint *>(cons->constraint.get())->setDamping(axis, damping);
            dynamic_cast<btGeneric6DofSpringConstraint *>(cons->constraint.get())->setEquilibriumPoint(axis, epVal);
        }
        else if (constraintType == "Generic6DofSpring2") {
            dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->enableSpring(axis, enabled);
            dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->setEquilibriumPoint(axis, epVal);
            dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->setDamping(axis, damping);
            dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->setStiffness(axis, stiffness);
        }
        else if (constraintType == "Hinge2") {
            dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->enableSpring(axis, enabled);
            dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->setEquilibriumPoint(axis, epVal);
            dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->setDamping(axis, damping);
            dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->setStiffness(axis, stiffness);
        }
        set_output("constraint", std::move(cons));
    }
};

ZENDEFNODE(BulletConstraintSetSpring , {
    {{gParamType_Unknown, "constraint"}, {gParamType_Bool, "enable", "true"}, {gParamType_Float, "stiffness"}, {gParamType_Float, "damping"}, {gParamType_Float, "equilibriumPointVal"}},
    {{gParamType_Unknown, "constraint"}},
    {{"enum Fixed Generic6DofSpring Generic6DofSpring2 Hinge2", "constraintType", "Fixed"}, {"enum linearX linearY linearZ angularX angularY angularZ", "axisId", "linearX"}},
    {"Bullet"},
});

struct BulletConstraintSetMotor : zeno::INode {
    virtual void apply() override {
        auto cons = safe_uniqueptr_cast<BulletConstraint>(move_input("constraint"));
        auto constraintType = zsString2Std(get_input2_string("constraintType"));
        auto axisId = zsString2Std(get_input2_string("axisId"));
        int axis;
        if (axisId == "linearX"){axis = 0;}
        else if (axisId == "linearY"){axis = 1;}
        else if (axisId == "linearZ"){axis = 2;}
        else if (axisId == "angularX"){axis = 3;}
        else if (axisId == "angularY"){axis = 4;}
        else {axis=5;} // "angularZ"

        auto bounce = btScalar(get_input2_float("bounce"));
        auto enableMotor = get_input2_bool("enableMotor");
        auto enableServo = get_input2_bool("enableServo");
        auto maxMotorForce = btScalar(get_input2_float("maxMotorForce"));
        auto servoTarget = btScalar(get_input2_float("servoTarget"));
        auto targetVelocity = btScalar(get_input2_float("targetVelocity"));

        auto maxMotorImpulse = btScalar(get_input2_float("maxMotorImpulse"));
        auto maxMotorImpulseNormalized = btScalar(get_input2_float("maxMotorImpulseNormalized"));
        auto motorTarget = zeno::vec_to_other<btQuaternion>(toVec4f(get_input2_vec4f("motorTarget")));
        auto motorTargetConstraint = zeno::vec_to_other<btQuaternion>(toVec4f(get_input2_vec4f("motorTargetConstraint")));
        auto angularOnly = get_input2_bool("angularOnly");
        auto fixThresh = btScalar(get_input2_float("fixThresh"));

        auto dt = btScalar(get_input2_float("dt"));
        if (constraintType == "ConeTwist") {
            dynamic_cast<btConeTwistConstraint *>(cons->constraint.get())->enableMotor(enableMotor);
            dynamic_cast<btConeTwistConstraint *>(cons->constraint.get())->setMaxMotorImpulse(maxMotorImpulse);
            dynamic_cast<btConeTwistConstraint *>(cons->constraint.get())->setMaxMotorImpulseNormalized(maxMotorImpulseNormalized);
            dynamic_cast<btConeTwistConstraint *>(cons->constraint.get())->setMotorTarget(motorTarget);
            dynamic_cast<btConeTwistConstraint *>(cons->constraint.get())->setMotorTargetInConstraintSpace(motorTargetConstraint);
            dynamic_cast<btConeTwistConstraint *>(cons->constraint.get())->setAngularOnly(angularOnly);

            dynamic_cast<btConeTwistConstraint *>(cons->constraint.get())->setFixThresh(fixThresh);
        }
		else if (constraintType == "Generic6Dof") {
	        dynamic_cast<btGeneric6DofConstraint *>(cons->constraint.get())->getTranslationalLimitMotor()->m_enableMotor[axis] = enableMotor;
	        dynamic_cast<btGeneric6DofConstraint *>(cons->constraint.get())->getTranslationalLimitMotor()->m_targetVelocity[axis] = targetVelocity;
	        dynamic_cast<btGeneric6DofConstraint *>(cons->constraint.get())->getTranslationalLimitMotor()->m_maxMotorForce[axis] = maxMotorForce;
		}
        else if (constraintType == "Generic6DofSpring2") {
            dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->setBounce(axis, bounce);
            dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->enableMotor(axis, enableMotor);
            dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->setServo(axis, enableServo);
            dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->setMaxMotorForce(axis, maxMotorForce);
            dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->setServoTarget(axis, servoTarget);
            dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->setTargetVelocity(axis, targetVelocity);
        }
        else if (constraintType == "Hinge")
        {
            dynamic_cast<btHingeConstraint *>(cons->constraint.get())->setAngularOnly(angularOnly);
            dynamic_cast<btHingeConstraint *>(cons->constraint.get())->setMaxMotorImpulse(maxMotorImpulse);
            dynamic_cast<btHingeConstraint *>(cons->constraint.get())->setMotorTarget(motorTarget, dt);
            dynamic_cast<btHingeConstraint *>(cons->constraint.get())->setMotorTargetVelocity(targetVelocity);
        }
        else if (constraintType == "Hinge2") {
            dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->setBounce(axis, bounce);
            dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->enableMotor(axis, enableMotor);
            dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->setServo(axis, enableServo);
            dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->setMaxMotorForce(axis, maxMotorForce);
            dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->setServoTarget(axis, servoTarget);
            dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->setTargetVelocity(axis, targetVelocity);
        }
        set_output("constraint", std::move(cons));
    }
};

ZENDEFNODE(BulletConstraintSetMotor , {
    {{gParamType_Unknown, "constraint"}, {gParamType_Float,"bounce","0"}, {gParamType_Bool, "enableMotor", "1"}, {gParamType_Bool,"enableServo","1"}, {gParamType_Float, "maxMotorForce", "0"}, {gParamType_Float,"servoTarget","0"}, {gParamType_Float, "targetVelocity", "0"}, {gParamType_Float,"maxMotorImpulse","0"}, {gParamType_Float,"maxMotorImpulseNormalized","0"}, {gParamType_Vec4f,"motorTarget","0,0,0,1"}, {gParamType_Vec4f,"motorTargetConstraint","0,0,0,1"}, {gParamType_Float,"angularOnly","1"}, {gParamType_Float,"fixThresh","0"}, {gParamType_Float,"dt","0"}},
    {{gParamType_Unknown, "constraint"}},
    {{"enum ConeTwist Generic6Dof Generic6DofSpring2 Hinge Hinge2", "constraintType", "ConeTwist"}, {"enum linearX linearY linearZ angularX angularY angularZ", "axisId", "linearX"}},
    {"Bullet"},
});

struct BulletConstraintSetRotOrder : zeno::INode {
    virtual void apply() override {
        auto cons = safe_uniqueptr_cast<BulletConstraint>(move_input("constraint"));
        auto constraintType = zsString2Std(get_input2_string("constraintType"));
        auto rotateOrder = zsString2Std(get_input2_string("rotateOrder"));

        RotateOrder rotOrder;
        if (rotateOrder == "XYZ"){
            rotOrder = RO_XYZ;
        }
        else if (rotateOrder == "XZY"){
            rotOrder = RO_XZY;
        }
        else if (rotateOrder == "YXZ"){
            rotOrder = RO_YXZ;
        }
        else if (rotateOrder == "YZX") {
            rotOrder = RO_YZX;
        }
        else if (rotateOrder == "ZXY") {
            rotOrder = RO_ZXY;
        }
        else {
            rotOrder = RO_ZYX;
        }

        if (constraintType == "Generic6DofSpring2") {
            dynamic_cast<btGeneric6DofSpring2Constraint *>(cons->constraint.get())->setRotationOrder(rotOrder);
        }
        else if (constraintType == "Hinge2") {
            dynamic_cast<btHinge2Constraint *>(cons->constraint.get())->setRotationOrder(rotOrder);
        }
        set_output("constraint", std::move(cons));
    }
};

ZENDEFNODE(BulletConstraintSetRotOrder, {
    {{gParamType_Unknown, "constraint"}},
    {{gParamType_Unknown, "constraint"}},
    {{"enum Generic6DofSpring2 Hinge2", "constraintType", "Hinge2"}, {"enum XYZ XZY YXZ YZX ZXY ZYX", "rotateOrder", "XYZ"}},
    {"Bullet"},
});


struct BulletGearConstraintSetRatio : zeno::INode {
    virtual void apply() override {
        auto cons = safe_uniqueptr_cast<BulletConstraint>(move_input("constraint"));
        auto ratio = btScalar(get_input2_float("ratio"));
        dynamic_cast<btGearConstraint *>(cons->constraint.get())->setRatio(ratio);
        set_output("constraint", std::move(cons));
    }
};

ZENDEFNODE(BulletGearConstraintSetRatio, {
    {{gParamType_Unknown, "constraint"}, {gParamType_Float, "ratio", "1"}},
    {{gParamType_Unknown, "constraint"}},
    {},
    {"Bullet"},
});


struct BulletSliderConstraintSetSpring : zeno::INode {
    virtual void apply() override {
        auto cons = safe_uniqueptr_cast<BulletConstraint>(move_input("constraint"));
        auto dampingDirAng = btScalar(get_input2_float("dampingDirAng"));
        auto dampingDirLin = btScalar(get_input2_float("dampingDirLin"));
        auto dampingLimAng = btScalar(get_input2_float("dampingLimAng"));
        auto dampingLimLin = btScalar(get_input2_float("dampingLimLin"));
        auto dampingOrthoAng = btScalar(get_input2_float("dampingOrthoAng"));
        auto dampingOrthoLin = btScalar(get_input2_float("dampingOrthoLin"));
        auto maxAngMotorForce = btScalar(get_input2_float("maxAngMotorForce"));
        auto maxLinMotorForce = btScalar(get_input2_float("maxLinMotorForce"));
        auto poweredAngMotor = get_input2_bool("poweredAngMotor");
        auto poweredLinMotor = get_input2_bool("poweredLinMotor");
        auto restitutionDirAng = btScalar(get_input2_float("restitutionDirAng"));
        auto restitutionDirLin = btScalar(get_input2_float("restitutionDirLin"));
        auto restitutionLimAng = btScalar(get_input2_float("restitutionLimAng"));
        auto restitutionLimLin = btScalar(get_input2_float("restitutionLimLin"));
        auto restitutionOrthoAng = btScalar(get_input2_float("restitutionOrthoAng"));
        auto restitutionOrthoLin = btScalar(get_input2_float("restitutionOrthoLin"));
        auto softnessDirAng = btScalar(get_input2_float("softnessDirAng"));
        auto softnessDirLin = btScalar(get_input2_float("softnessDirLin"));
        auto softnessLimAng = btScalar(get_input2_float("softnessLimAng"));
        auto softnessLimLin = btScalar(get_input2_float("softnessLimLin"));
        auto softnessOrthoAng = btScalar(get_input2_float("softnessOrthoAng"));
        auto softnessOrthoLin = btScalar(get_input2_float("softnessOrthoLin"));
        auto targetAngMotorVelocity = btScalar(get_input2_float("targetAngMotorVelocity"));
        auto targetLinMotorVelocity = btScalar(get_input2_float("targetLinMotorVelocity"));

        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setDampingDirAng(dampingDirAng);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setDampingDirLin(dampingDirLin);

        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setDampingLimAng(dampingLimAng);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setDampingLimLin(dampingLimLin);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setDampingOrthoAng(dampingOrthoAng);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setDampingOrthoLin(dampingOrthoLin);

        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setMaxAngMotorForce(maxAngMotorForce);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setMaxLinMotorForce(maxLinMotorForce);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setPoweredAngMotor(poweredAngMotor);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setPoweredLinMotor(poweredLinMotor);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setRestitutionDirAng(restitutionDirAng);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setRestitutionDirLin(restitutionDirLin);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setRestitutionLimAng(restitutionLimAng);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setRestitutionLimLin(restitutionLimLin);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setRestitutionOrthoAng(restitutionOrthoAng);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setRestitutionOrthoLin(restitutionOrthoLin);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setSoftnessDirAng(softnessDirAng);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setSoftnessDirLin(softnessDirLin);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setSoftnessLimAng(softnessLimAng);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setSoftnessLimLin(softnessLimLin);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setSoftnessOrthoAng(softnessOrthoAng);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setSoftnessOrthoLin(softnessOrthoLin);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setTargetAngMotorVelocity(targetAngMotorVelocity);
        dynamic_cast<btSliderConstraint *>(cons->constraint.get())->setTargetLinMotorVelocity(targetLinMotorVelocity);

        set_output("constraint", std::move(cons));
    }
};

ZENDEFNODE(BulletSliderConstraintSetSpring, {
    {
        {gParamType_Unknown, "constraint"},
        {gParamType_Float, "dampingDirAng"},
        {gParamType_Float, "dampingDirLin"},
        {gParamType_Float, "dampingLimAng"},
        {gParamType_Float, "dampingLimLin"},
        {gParamType_Float, "dampingOrthoAng"},
        {gParamType_Float, "dampingOrthoLin"},
        {gParamType_Float, "maxAngMotorForce"},
        {gParamType_Float, "maxLinMotorForce"},
        {gParamType_Float, "poweredAngMotor"},
        {gParamType_Float, "poweredLinMotor"},
        {gParamType_Float, "restitutionDirAng"},
        {gParamType_Float, "poweredLinMotor"},
        {gParamType_Float, "restitutionDirAng"},
        {gParamType_Float, "restitutionDirLin"},
        {gParamType_Float, "restitutionLimAng"},
        {gParamType_Float, "restitutionLimLin"},
        {gParamType_Float, "restitutionOrthoAng"},
        {gParamType_Float, "restitutionOrthoLin"},
        {gParamType_Float, "softnessDirAng"},
        {gParamType_Float, "softnessDirLin"},
        {gParamType_Float, "softnessLimAng"},
        {gParamType_Float, "softnessLimLin"},
        {gParamType_Float, "softnessOrthoAng"},
        {gParamType_Float, "softnessOrthoLin"},
        {gParamType_Float, "targetAngMotorVelocity"},
        {gParamType_Float, "targetLinMotorVelocity"},
    },
    {{gParamType_Unknown, "constraint"}},
    {},
    {"Bullet"},
});

/*
 *  Bullet World
 */


struct BulletWorldAddObject : zeno::INode {
    virtual void apply() override {
        auto world = safe_uniqueptr_cast<BulletWorldProxy>(clone_input("world"));
        auto object = safe_uniqueptr_cast<BulletObjectProxy>(clone_input("object"));
        world->addObject(std::move(object));
        set_output("world", get_input("world"));
    }
};

ZENDEFNODE(BulletWorldAddObject, {
                                     {{gParamType_Unknown, "world"}, {gParamType_Unknown, "object"}},
                                     {{gParamType_Unknown, "world"}},
                                     {},
                                     {"Bullet"},
                                 });

struct BulletWorldRemoveObject : zeno::INode {
    virtual void apply() override {
        auto world = safe_uniqueptr_cast<BulletWorldProxy>(clone_input("world"));
        auto object = safe_uniqueptr_cast<BulletObjectProxy>(clone_input("object"));
        world->removeObject(std::move(object));
        set_output("world", get_input("world"));
    }
};

ZENDEFNODE(BulletWorldRemoveObject, {
                                        {{gParamType_Unknown, "world"}, {gParamType_Unknown, "object"}},
                                        {{gParamType_Unknown, "world"}},
                                        {},
                                        {"Bullet"},
                                    });

struct BulletWorldAddConstraint : zeno::INode {
    virtual void apply() override {
        auto world = safe_uniqueptr_cast<BulletWorldProxy>(clone_input("world"));
        auto constraint = safe_uniqueptr_cast<BulletConstraint>(move_input("constraint"));
        world->addConstraint(constraint);
        set_output("world", get_input("world"));
    }
};

ZENDEFNODE(BulletWorldAddConstraint, {
                                         {{gParamType_Unknown, "world"}, {gParamType_Unknown, "constraint"}},
                                         {{gParamType_Unknown, "world"}},
                                         {},
                                         {"Bullet"},
                                     });

struct BulletWorldRemoveConstraint : zeno::INode {
    virtual void apply() override {
        auto world = safe_uniqueptr_cast<BulletWorldProxy>(clone_input("world"));
        auto constraint = safe_uniqueptr_cast<BulletConstraint>(move_input("constraint"));
        world->removeConstraint(std::move(constraint));
        set_output("world", get_input("world"));
    }
};

ZENDEFNODE(BulletWorldRemoveConstraint, {
                                            {{gParamType_Unknown, "world"}, {gParamType_List, "consList"}},
                                            {{gParamType_Unknown, "world"}},
                                            {},
                                            {"Bullet"},
                                        });




/*
 * Bullet MultiBody
 */



struct BulletMultiBodyObjectMakeStart : zeno::INode {
    virtual void apply() override {
	auto n_links = get_input2_int("nLinks");
	auto mass = get_input2_float("mass");
	btVector3 inertia(0.f, 0.f, 0.f);
	if (has_input("inertia"))
		inertia = zeno::vec_to_other<btVector3>(toVec3f(get_input2_vec3f("inertia")));
	auto fixedBase = (zsString2Std(get_input2_string("fixedBase")) == "true");
	auto canSleep = (zsString2Std(get_input2_string("canSleep")) == "true");
	auto object = std::make_unique<BulletMultiBodyObject>(n_links, mass, inertia, fixedBase, canSleep);

	set_output("object", std::move(object));
    }
};

ZENDEFNODE(BulletMultiBodyObjectMakeStart, {
    {{gParamType_Int, "nLinks"}, {gParamType_Float, "mass", "0"}, {gParamType_Vec3f, "inertia"}},
	{{gParamType_Unknown, "object"}},
	{{"enum true false", "fixedBase", "true"}, {"enum true false", "canSleep", "true"}},
	{"Bullet"}
});

struct BulletMultiBodyObjectMakeEnd : zeno::INode {
    virtual void apply() override {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        object->multibody->finalizeMultiDof();
        set_output("object", std::move(object));
    }
};

ZENDEFNODE(BulletMultiBodyObjectMakeEnd, {
	{{gParamType_Unknown, "object"}},
	{{gParamType_Unknown, "object"}},
	{},
	{"Bullet"}
});

struct BulletMultiBodySetCollider : zeno::INode {
    virtual void apply() override {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        auto link_id = get_input2_int("linkIndex");
        auto collider = safe_uniqueptr_cast<BulletMultiBodyLinkCollider>(move_input("collider"));
        if (link_id < 0) {
                object->multibody->setBaseCollider(collider->linkCollider.get());
        }
        else{
                object->multibody->getLink(link_id).m_collider = collider->linkCollider.get();
        }
        set_output("object", std::move(object));
    }
};

ZENDEFNODE(BulletMultiBodySetCollider, {
	{{gParamType_Unknown, "object"}, {gParamType_Unknown, "linkIndex"}, {gParamType_Unknown, "collider"}},
	{{gParamType_Unknown, "object"}},
	{},
	{"Bullet"},
});

struct BulletMultiBodySetupJoint : zeno::INode {
    virtual void apply() override {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        auto jointType = zsString2Std(get_input2_string("jointType"));
        auto i = get_input2_int("linkIndex");
        auto parent = get_input2_int("parentIndex");
        auto mass = get_input2_float("mass");
        auto inertia = zeno::vec_to_other<btVector3>(toVec3f(get_input2_vec3f("inertia")));
        auto jointAxis = zeno::vec_to_other<btVector3>(toVec3f(get_input2_vec3f("jointAxis")));
        auto rotParentToThis = zeno::vec_to_other<btQuaternion>(toVec4f(get_input2_vec4f("rotParentToThis")));
        auto parentComToThisPivotOffset = zeno::vec_to_other<btVector3>(toVec3f(get_input2_vec3f("parentComToThisPivotOffset")));
        auto thisPivotToThisComOffset = zeno::vec_to_other<btVector3>(toVec3f(get_input2_vec3f("thisPivotToThisComOffset")));
        auto disableParentCollision= (zsString2Std(get_input2_string("disableParentCollision")) == "true");

        if (jointType == "Fixed") {
                object->multibody->setupFixed(i, mass, inertia, parent, rotParentToThis, parentComToThisPivotOffset, thisPivotToThisComOffset, disableParentCollision);
        }
        else if (jointType == "Prismatic") {
                object->multibody->setupPrismatic(i, mass, inertia, parent, rotParentToThis, jointAxis, parentComToThisPivotOffset, thisPivotToThisComOffset, disableParentCollision);
        }
        else if (jointType == "Revolute") {
                object->multibody->setupRevolute(i, mass, inertia, parent, rotParentToThis, jointAxis, parentComToThisPivotOffset, thisPivotToThisComOffset, disableParentCollision);
        }
        else if (jointType == "Spherical") {
                object->multibody->setupSpherical(i, mass, inertia, parent, rotParentToThis, parentComToThisPivotOffset, thisPivotToThisComOffset, disableParentCollision);
        }
        else { // planar
                object->multibody->setupPlanar(i, mass, inertia, parent, rotParentToThis, jointAxis, parentComToThisPivotOffset, disableParentCollision);
        }

        set_output("object", std::move(object));
    }
};

ZENDEFNODE(BulletMultiBodySetupJoint, {
    {{gParamType_Unknown, "object"},
     {gParamType_Unknown, "linkIndex"},
    {gParamType_Int, "parentIndex"},
    {gParamType_Float, "mass"},
    {gParamType_Vec3f, "inertia"},
    {gParamType_Vec3f, "jointAxis"},
    {gParamType_Vec4f, "rotParentToThis"},
    {gParamType_Vec3f, "parentComToThisPivotOffset"},
    {gParamType_Vec3f, "thisPivotToThisComOffset"}},
	{{gParamType_Unknown, "object"}},
	{{"enum Fixed Prismatic Revolute Spherical Planar", "jointType", "Revolute"}, {"enum true false", "disableParentCollision", "false"}},
	{"Bullet"}
});

struct BulletMultiBodySetJointProperty : zeno::INode{
	virtual void apply() override {
		auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
		auto link_id = get_input2_int("linkIndex");
		if(has_input("damping")){
			auto damping = get_input2_float("damping");
			object->multibody->getLink(link_id).m_jointDamping = damping;
		}
		if(has_input("friction")){
			auto friction = get_input2_float("friction");
			object->multibody->getLink(link_id).m_jointFriction = friction;
		}
		if(has_input("lowerLimit")){
			auto lowerLimit = get_input2_float("lowerLimit");
			object->multibody->getLink(link_id).m_jointLowerLimit = lowerLimit;
		}
		if(has_input("upperLimit")){
			auto upperLimit = get_input2_float("upperLimit");
			object->multibody->getLink(link_id).m_jointUpperLimit = upperLimit;
		}
		if(has_input("maxForce")){
			auto maxForce = get_input2_float("maxForce");
			object->multibody->getLink(link_id).m_jointMaxForce = maxForce;
		}
		if(has_input("maxVelocity")){
			auto maxVelocity = get_input2_float("maxVelocity");
			object->multibody->getLink(link_id).m_jointMaxVelocity = maxVelocity;
		}
		set_output("object", std::move(object));
	}
};

ZENDEFNODE(BulletMultiBodySetJointProperty, {
    {{gParamType_Unknown, "object"}, {gParamType_Int, "linkIndex", "0"}, {gParamType_Float, "damping"}, {gParamType_Float, "friction"}, {gParamType_Float, "lowerLimit"}, {gParamType_Float, "upperLimit"}, {gParamType_Float, "maxForce"}, {gParamType_Float, "maxVelocity"}},
	{{gParamType_Unknown, "object"}},
	{},
	{"Bullet"}
});

struct BulletMultiBodySetBaseTransform : zeno::INode {
	virtual void apply() override {
		auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
		auto trans = safe_uniqueptr_cast<BulletTransform>(move_input("baseTrans"));

		object->multibody->setBaseWorldTransform(trans->trans);
		set_output("object", std::move(object));
	}
};

ZENDEFNODE(BulletMultiBodySetBaseTransform, {
	{{gParamType_Unknown, "object"}, {gParamType_Unknown, "baseTrans"}},
	{{gParamType_Unknown, "object"}},
	{},
	{"Bullet"}
});

struct BulletMultiBodyLinkGetTransform : zeno::INode {
	virtual void apply() override {
		auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
		auto link_id = get_input2_int("linkIndex");
        auto trans = std::make_unique<BulletTransform>();
        trans->trans = object->multibody->getLink(link_id).m_collider->getWorldTransform();
		set_output("trans", std::move(trans));
	}
};

ZENDEFNODE(BulletMultiBodyLinkGetTransform, {
	{{gParamType_Unknown, "object"}, {gParamType_Int, "linkIndex"}},
	{{gParamType_Unknown, "trans"}},
	{},
	{"Bullet"}
});

struct BulletMultiBodyGetTransform : zeno::INode {
    virtual void apply() override {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        auto transList = create_ListObject();
        transList->clear();

        for (size_t i = 0; i < object->multibody->getNumLinks(); i++) {
            auto trans = std::make_unique<BulletTransform>();
            trans->trans = object->multibody->getLink(i).m_collider->getWorldTransform();
            std::cout<< "\nlink #" << i << ": " << trans->trans.getOrigin()[0] << "," << trans->trans.getOrigin()[1] << "," << trans->trans.getOrigin()[2] << "\n";
            std::cout << trans->trans.getRotation()[0] << "," << trans->trans.getRotation()[1] << "," << trans->trans.getRotation()[2] << "," << trans->trans.getRotation()[3] << std::endl;
            transList->push_back(trans);
        }
        set_output("transList", std::move(transList));
    }
};

ZENDEFNODE(BulletMultiBodyGetTransform, {
    {{gParamType_Unknown, "object"}},
    {{gParamType_List, "transList"}},
    {},
    {"Bullet"}
});

struct BulletMultiBodyWorldGetTransformShapes : zeno::INode {
	virtual void apply() override {
		auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
        auto graphicsVisualMap = get_input_DictObject("visualMap");

        auto transList = create_ListObject();
        transList->clear();

        auto visualList = create_ListObject();
        visualList->clear();

        int numCollisionObjects = world->dynamicsWorld->getNumCollisionObjects();
        for (size_t i = 0; i < numCollisionObjects; i++) {
            auto linkTrans = std::make_unique<BulletTransform>();
            btCollisionObject* colObj = world->dynamicsWorld->getCollisionObjectArray()[i];
            btCollisionShape* collisionShape = colObj->getCollisionShape();
            linkTrans->trans = colObj->getWorldTransform();
            //linkTrans->trans = btTransform::getIdentity();
            int graphicsIndex = colObj->getUserIndex();
            std::cout << "graphicsId #" << graphicsIndex << ":" << linkTrans->trans.getOrigin()[0] << "," << linkTrans->trans.getOrigin()[1] << "," << linkTrans->trans.getOrigin()[2] << "\n";
            std::cout << linkTrans->trans.getRotation()[0] << "," << linkTrans->trans.getRotation()[1] << "," << linkTrans->trans.getRotation()[2] << "," << linkTrans->trans.getRotation()[3] << std::endl;

            if (graphicsIndex >= 0) {
                transList->push_back(linkTrans);
                visualList->push_back(graphicsVisualMap->lut.at(std::to_string(graphicsIndex)));
            }
        }

        set_output("transList", std::move(transList));
        set_output("visualList", std::move(visualList));
	}
};

ZENDEFNODE(BulletMultiBodyWorldGetTransformShapes, {
    {{gParamType_Unknown, "world"}, {gParamType_Dict, "visualMap"}},
    {{gParamType_List, "transList"}, {gParamType_List, "visualList"}},
	{},
	{"Bullet"}
});

struct BulletMultiBodySetProperty : zeno::INode {
	virtual void apply() override {
		auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
		auto canSleep = (zsString2Std(get_input2_string("canSleep")) == "true");
		auto selfCollide = (zsString2Std(get_input2_string("selfCollide")) == "true");
		auto useGyro = (zsString2Std(get_input2_string("useGyro")) == "true");
		auto linearDamping = get_input2_float("linearDamp");
		auto angularDamping = get_input2_float("angularDamp");

		object->multibody->setCanSleep(canSleep);
		object->multibody->setHasSelfCollision(selfCollide);
		object->multibody->setUseGyroTerm(useGyro);
		object->multibody->setLinearDamping(linearDamping);
		object->multibody->setAngularDamping(angularDamping);

		set_output("object", std::move(object));
	}
};

ZENDEFNODE(BulletMultiBodySetProperty, {
	{{gParamType_Unknown, "object"}, {gParamType_Float, "linearDamp", "0"}, {gParamType_Float, "angularDamp", "0"}},
	{{gParamType_Unknown, "object"}},
	{{"enum true false", "canSleep", "false"},
		{"enum true false", "selfCollide", "false"},
		{"enum true false", "useGyro", "false"}},
	{"Bullet"}
});


struct BulletMultiBodySetCollisionShapeForLink : zeno::INode {
	virtual void apply() override {
		auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
		auto link_id = get_input2_int("linkIndex");
		auto collisionShape = safe_uniqueptr_cast<BulletCollisionShapeProxy>(clone_input("colShape"));
		auto col = std::make_unique<BulletMultiBodyLinkCollider>(object->multibody.get(), link_id);
		col->linkCollider->setCollisionShape(collisionShape->shape.get());
        if (link_id < 0) {
            object->multibody->setBaseCollider(col->linkCollider.get());
        }
        else {
            object->multibody->getLink(
                link_id).m_collider = col->linkCollider.get();
        }
        set_output("object", std::move(object));
		set_output("collider", std::move(col));
	}
};

ZENDEFNODE(BulletMultiBodySetCollisionShapeForLink, {
	{{gParamType_Unknown, "object"}, {gParamType_Int, "linkIndex"}, {gParamType_Unknown, "colShape"}},
	{{gParamType_Unknown, "object"}, {gParamType_Unknown, "collider"}},
	{},
	{"Bullet"},
});

struct BulletMultiBodyLinkColliderSetTransform : zeno::INode {
	virtual void apply() override {
		auto col = safe_uniqueptr_cast<BulletMultiBodyLinkCollider>(move_input("collider"));
		auto trans = safe_uniqueptr_cast<BulletTransform>(move_input("trans"));

		col->linkCollider->setWorldTransform(trans->trans);
		set_output("collider", std::move(col));
	}
};

ZENDEFNODE(BulletMultiBodyLinkColliderSetTransform, {
	{{gParamType_Unknown, "collider"}, {gParamType_Unknown, "trans"}},
	{{gParamType_Unknown, "collider"}},
	{},
	{"Bullet"},
});

struct BulletMultiBodyForwardKinematics : zeno::INode {
	virtual void apply() override {
		auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
		btAlignedObjectArray<btQuaternion> scratch_q;
		btAlignedObjectArray<btVector3> scratch_m;
		object->multibody->forwardKinematics(scratch_q, scratch_m);
		set_output("object", std::move(object));
	}
};

ZENDEFNODE(BulletMultiBodyForwardKinematics, {
	{{gParamType_Unknown, "object"}},
	{{gParamType_Unknown, "object"}},
	{},
	{"Bullet"},
});

struct BulletMultiBodyUpdateColObjectTransform : zeno::INode {
	virtual void apply() override {
		auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
		btAlignedObjectArray<btQuaternion> world_to_local;
		btAlignedObjectArray<btVector3> local_origin;
		object->multibody->updateCollisionObjectWorldTransforms(world_to_local, local_origin);
		set_output("object", std::move(object));
	}
};

ZENDEFNODE(BulletMultiBodyUpdateColObjectTransform, {
	{{gParamType_Unknown, "object"}},
	{{gParamType_Unknown, "object"}},
	{},
	{"Bullet"},
});

struct BulletMultiBodyAddJointTorque : zeno::INode {
	virtual void apply() override {
		auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
		auto link_id = get_input2_int("linkIndex");
		auto torque = get_input2_float("torque");

		object->multibody->addJointTorque(link_id, torque);
		set_output("object", std::move(object));
	}
};

ZENDEFNODE(BulletMultiBodyAddJointTorque, {
    {{gParamType_Unknown, "object"}, {gParamType_Int, "linkIndex"}, {gParamType_Float, "torque"}},
	{{gParamType_Unknown, "object"}},
	{},
	{"Bullet"},
});

struct BulletMultiBodySetJointPosMultiDof : zeno::INode {
	virtual void apply() override {
		auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
		auto link_id = get_input2_int("startIndex");
		auto isSpherical = (zsString2Std(get_input2_string("isSpherical")) == "true");
		if (isSpherical){
			auto pos = zeno::vec_to_other<btQuaternion>(toVec4f(get_input2_vec4f("pos")));
			pos.normalize();
			object->multibody->setJointPosMultiDof(link_id, pos);
		}
		else{
			auto pos = get_input2_float("pos");
			object->multibody->setJointPosMultiDof(link_id, &pos);
		}

		set_output("object", std::move(object));
	}
};

ZENDEFNODE(BulletMultiBodySetJointPosMultiDof, {
	{{gParamType_Unknown, "object"}, {gParamType_Int, "startIndex", "0"}, {gParamType_Float, "pos", ""}},
	{{gParamType_Unknown, "object"}},
	{{"enum true false", "isSpherical", "false"}},
	{"Bullet"},
});

struct BulletMultiBodySetJointVelMultiDof : zeno::INode {
	virtual void apply() override {
		auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
		auto link_id = get_input2_int("startIndex");
		auto isSpherical = (zsString2Std(get_input2_string("isSpherical")) == "true");
		if (isSpherical){
			auto pos = zeno::vec_to_other<btQuaternion>(toVec4f(get_input2_vec4f("pos")));
			pos.normalize();
			object->multibody->setJointVelMultiDof(link_id, pos);
		}
		else{
			auto pos = get_input2_float("pos");
			object->multibody->setJointVelMultiDof(link_id, &pos);
		}

		set_output("object", std::move(object));
	}
};

ZENDEFNODE(BulletMultiBodySetJointVelMultiDof, {
    {{gParamType_Unknown, "object"}, {gParamType_Int, "startIndex", "0"}, {gParamType_Float, "pos", ""}},
	{{gParamType_Unknown, "object"}},
	{{"enum true false", "isSpherical", "false"}},
	{"Bullet"},
});


struct BulletMultiBodySetJointFeedback : zeno::INode {
	virtual void apply() override {
		auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));

		auto fb = std::make_shared<MultiBodyJointFeedback>();

		for (int i = 0; i < object->multibody->getNumLinks(); i++)
		{
			object->multibody->getLink(i).m_jointFeedback = &fb->jointFeedback;
			object->jointFeedbacks.push_back(&fb->jointFeedback);
		}

		set_output("object", std::move(object));
	}
};

ZENDEFNODE(BulletMultiBodySetJointFeedback, {
	{{gParamType_Unknown, "object"}},
	{{gParamType_Unknown, "object"}},
	{},
	{"Bullet"},
});

struct BulletMultiBodyPDControl : zeno::INode {
	virtual void apply() override {
		auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
		auto kp = get_input2_float("kp");
		auto kd = get_input2_float("kd");
		auto maxForce = get_input2_float("maxForce");

		btAlignedObjectArray<btScalar> qDesiredArray;
		btAlignedObjectArray<btScalar> qdDesiredArray;

        if (has_input("qDesiredList")){
            {
                auto numericObjs = get_input_ListObject("qDesiredList")->get<std::decay_t<NumericObject>>();
                for (auto &&no: numericObjs)
                    qDesiredArray.push_back(no->get<float>());
            }
		}
        else{
            qDesiredArray.resize(object->multibody->getNumLinks(), 0);
        }

		if (has_input("dqDesiredList")) {
            {
                auto numericObjs = get_input_ListObject("dqDesiredList")->get<std::decay_t<NumericObject>>();
                for (auto &&no: numericObjs)
                    qdDesiredArray.push_back(no->get<float>());
            }
		}
        else {
            qdDesiredArray.resize(object->multibody->getNumLinks(), 0);
        }

		for (int joint = 0; joint < object->multibody->getNumLinks(); joint++)
		{
			int dof1 = 0;
			btScalar qActual = object->multibody->getJointPosMultiDof(joint)[dof1];
			btScalar qdActual = object->multibody->getJointVelMultiDof(joint)[dof1];
			btScalar positionError = (qDesiredArray[joint] - qActual);
			btScalar velocityError = (qdDesiredArray[joint] - qdActual);
			btScalar force = kp * positionError + kd * velocityError;
			btClamp(force, -maxForce, maxForce);
            std::cout << "current force for link #" << joint << " is " << force << std::endl;
			object->multibody->addJointTorque(joint, force);
		}
	}
};

ZENDEFNODE(BulletMultiBodyPDControl, {
	{{gParamType_Unknown, "object"}, {gParamType_Float, "kp", "100"}, {gParamType_Float, "kd", "20"}, {gParamType_Float, "maxForce", "100"}, {gParamType_List, "qDesiredList"}, {gParamType_List, "dqDesiredList"}},
	{},
	{},
	{"Bullet"},
});
/*
 * Bullet MultiBody World
 */

struct BulletMakeMultiBodyWorld : zeno::INode {
	virtual void apply() override {
		auto solverType = zsString2Std(get_input2_string("solverType"));
		auto world = std::make_unique<BulletMultiBodyWorld>(solverType);
		set_output("world", std::move(world));
	}
};

ZENDEFNODE(BulletMakeMultiBodyWorld, {
	{},
	{{gParamType_Unknown, "world"}},
    {{"enum SequentialImpulse ProjectedGaussSeidel Dantzig", "solverType", "SequentialImpulse"}},
	{"Bullet"},
});

struct BulletMultiBodyWorldSetGravity : zeno::INode {
	virtual void apply() override {
		auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
		auto gravity = toVec3f(get_input2_vec3f("gravity"));
		world->dynamicsWorld->setGravity(zeno::vec_to_other<btVector3>(gravity));
		set_output("world", std::move(world));
	}
};

ZENDEFNODE(BulletMultiBodyWorldSetGravity, {
	{{gParamType_Unknown, "world"}, {gParamType_Vec3f, "gravity", "0,0,-9.8"}},
	{{gParamType_Unknown, "world"}},
	{},
	{"Bullet"},
});

struct BulletStepMultiBodyWorld : zeno::INode {
	virtual void apply() override {
		auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
		auto dt = get_input2_float("dt");
		auto steps = get_input2_int("maxSubSteps");
        auto fixedTimeStep = get_input2_float("fixedTimeStep");

        world->dynamicsWorld->stepSimulation(dt, steps, fixedTimeStep);
		set_output("world", std::move(world));
	}
};

ZENDEFNODE(BulletStepMultiBodyWorld, {
	{{gParamType_Unknown, "world"}, {gParamType_Float, "dt", "0.04"}, {gParamType_Int, "maxSubSteps", "1"}, {gParamType_Float, "fixedTimeStep", "0.0042"}},
	{{gParamType_Unknown, "world"}},
	{},
	{"Bullet"},
});

struct BulletMultiBodyWorldAddObject : zeno::INode {
	virtual void apply() override {
		auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
        auto objType = zsString2Std(get_input2_string("objType"));
        if (objType == "multi") {
            auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
            world->dynamicsWorld->addMultiBody(object->multibody.get());
        }
        else {
            auto object = safe_uniqueptr_cast<BulletObjectProxy>(clone_input("object"));
            world->dynamicsWorld->addRigidBody(object->body.get());
        }
		set_output("world", std::move(world));
	}
};


ZENDEFNODE(BulletMultiBodyWorldAddObject, {
	{{gParamType_Unknown, "world"}, {gParamType_Unknown, "object"}},
	{{gParamType_Unknown, "world"}},
	{{"enum rigid multi", "objType", "multi"}},
	{"Bullet"},
});

struct BulletMultiBodyWorldRemoveObject : zeno::INode {
	virtual void apply() override {
		auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
        auto objType = zsString2Std(get_input2_string("objType"));

        if (objType == "multi") {
            auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
            world->dynamicsWorld->removeMultiBody(object->multibody.get());
        }
        else {
            auto object = safe_uniqueptr_cast<BulletObjectProxy>(clone_input("object"));
            world->dynamicsWorld->removeRigidBody(object->body.get());
        }

		set_output("world", get_input("world"));
	}
};

ZENDEFNODE(BulletMultiBodyWorldRemoveObject, {
	{{gParamType_Unknown, "world"}, {gParamType_Unknown, "object"}},
	{{gParamType_Unknown, "world"}},
	{{"enum rigid multi", "objType", "multi"}},
	{"Bullet"},
});


struct BulletMultiBodyWorldAddCollisionObject : zeno::INode {
	virtual void apply() override {
		auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
		auto isDynamic = (zsString2Std(get_input2_string("isDynamic")) == "true");
		auto col = safe_uniqueptr_cast<BulletMultiBodyLinkCollider>(move_input("collider"));
		int collisionFilterGroup = isDynamic ? int(btBroadphaseProxy::DefaultFilter) : int(btBroadphaseProxy::StaticFilter);
		int collisionFilterMask = isDynamic ? int(btBroadphaseProxy::AllFilter) : int(btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter);

        std::cout<< "add collider here: " << (void *)col.get() << std::endl;
		world->dynamicsWorld->addCollisionObject(col->linkCollider.get(), collisionFilterGroup, collisionFilterMask);

		set_output("world", world);
	}
};

ZENDEFNODE(BulletMultiBodyWorldAddCollisionObject, {
	{{gParamType_Unknown, "world"}, {gParamType_Unknown, "collider"}},
	{{gParamType_Unknown, "world"}},
	{{"enum true false", "isDynamic", "true"}},
	{"Bullet"},
});


struct BulletMultiBodyMakeConstraint : zeno::INode {
	virtual void apply() override {
		auto constraintType = zsString2Std(get_input2_string("constraintType"));
		auto bodyA = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("bodyA"));
		auto linkA = get_input2_int("linkA");

		std::map<std::string, btScalar> config;

		if (has_input("bodyB")) {
			auto bodyB = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("bodyB"));
			auto linkB = get_input2_int("linkB");
			auto cons = std::make_unique<BulletMultiBodyConstraint>(bodyA->multibody.get(), bodyB->multibody.get(), linkA, linkB, constraintType);
			set_output("constraint", cons);
		}
		else{
			if (has_input("lowerLimit")){
				auto lowerLimit = btScalar(get_input2_float("lowerLimit"));
				config["jointLowerLimit"] = lowerLimit;
			}
			if(has_input("upperLimit")) {
				auto upperLimit = btScalar(get_input2_float("upperLimit"));
				config["jointUpperLimit"] = upperLimit;
			}
			if(has_input("twistLimit")) {
				auto twistLimit = btScalar(get_input2_float("twistLimit"));
				config["jointTwistLimit"] = twistLimit;
			}
			if(has_input("jointMaxForce")) {
				auto jointMaxForce = btScalar(get_input2_float("jointMaxForce"));
				config["jointMaxForce"] = jointMaxForce;
			}
			if(has_input("desiredVelocity")) {
				auto desiredVelocity = btScalar(get_input2_float("desiredVelocity"));
				config["desiredVelocity"] = desiredVelocity;
			}
			auto cons = std::make_unique<BulletMultiBodyConstraint>(bodyA->multibody.get(), linkA, constraintType, config);
			set_output("constraint", cons);
		}
	}
};

ZENDEFNODE(BulletMultiBodyMakeConstraint, {
    {{gParamType_Unknown, "bodyA"}, {gParamType_Int, "linkA"}, {gParamType_Unknown, "bodyB"}, {gParamType_Int, "linkB"}, {gParamType_Float, "lowerLimit"}, {gParamType_Float, "upperLimit"}, {gParamType_Float,"twistLimit"}, {gParamType_Float,"jointMaxForce"}, {gParamType_Float,"desiredVelocity"}},
	{{gParamType_Unknown, "constraint"}},
	{{"enum Default DefaultMotor Spherical SphericalMotor Fixed Gear Point2Point Slider", "constraintType", "Default"}},
	{"Bullet"},
});

struct BulletMultiBodyWorldAddConstraint : zeno::INode {
	virtual void apply() override {
		auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
		auto constraint = safe_uniqueptr_cast<BulletMultiBodyConstraint>(move_input("constraint"));
		world->dynamicsWorld->addMultiBodyConstraint(constraint->constraint.get());
		set_output("world", get_input("world"));
	}
};

ZENDEFNODE(BulletMultiBodyWorldAddConstraint, {
    {{gParamType_Unknown, "world"}, {gParamType_Unknown, "constraint"}},
	{{gParamType_Unknown, "world"}},
	{},
	{"Bullet"},
});

struct BulletMultiBodyWorldRemoveConstraint : zeno::INode {
	virtual void apply() override {
		auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
		auto constraint = safe_uniqueptr_cast<BulletMultiBodyConstraint>(move_input("constraint"));
		world->dynamicsWorld->removeMultiBodyConstraint(constraint->constraint.get());
		set_output("world", get_input("world"));
	}
};

ZENDEFNODE(BulletMultiBodyWorldRemoveConstraint, {
    {{gParamType_Unknown, "world"}, {gParamType_Unknown, "constraint"}},
	{{gParamType_Unknown, "world"}},
	{},
	{"Bullet"},
});

struct BulletMultiBodyWorldAddConstraintEnd : zeno::INode {
    virtual void apply() override {
        auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
        for (int i = 0; i < world->dynamicsWorld->getNumMultiBodyConstraints(); i++)
        {
            world->dynamicsWorld->getMultiBodyConstraint(i)->finalizeMultiDof();
        }
        set_output("world", std::move(world));
    }
};

ZENDEFNODE(BulletMultiBodyWorldAddConstraintEnd, {
    {{gParamType_Unknown, "world"}},
    {{gParamType_Unknown, "world"}},
    {},
    {"Bullet"}
});
/*
 * Bullet Kinematics
 */

struct BulletCalcInverseKinematics : zeno::INode {
    virtual void apply() override {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        btVector3 gravity(0, -9.81, 0);
        if (has_input("gravity")) {
            gravity = zeno::vec_to_other<btVector3>(
                toVec3f(get_input2_vec3f("gravity")));
        }

        std::vector<int> endEffectorLinkIndices;
        {
            auto numericObjs = get_input_ListObject("endEffectorLinkIndices")->get<std::decay_t<NumericObject>>();
            for (auto &&no: numericObjs)
                endEffectorLinkIndices.push_back(no->get<int>());
        }

        auto numEndEffectorLinkIndices = endEffectorLinkIndices.size();

        std::vector<vec3f> targetPositions;
        {
            auto numericObjs = get_input_ListObject("targetPositions")->get<std::decay_t<NumericObject>>();
            for (auto &&no: numericObjs)
                targetPositions.push_back(no->get<zeno::vec3f>());
        }

        std::vector<vec4f> targetOrientations;
        {
            auto numericObjs = get_input_ListObject("targetOrientations")->get<std::decay_t<NumericObject>>();
            for (auto &&no : numericObjs)
                targetOrientations.push_back(no->get<zeno::vec4f>());
        }

		auto numIterations = get_input2_int("numIterations"); // 20
		auto residualThreshold = get_input2_float("residualThreshold"); // 1e-4

		auto ikMethod = zsString2Std(get_input2_string("IKMethod"));

		auto ikHelperPtr = std::make_shared<IKTrajectoryHelper>();

	    btAlignedObjectArray<double> startingPositions;
	    startingPositions.reserve(object->multibody->getNumLinks());

        {
		    for (int i = 0; i < object->multibody->getNumLinks(); ++i)
		    {
			    if (object->multibody->getLink(i).m_jointType >= 0 && object->multibody->getLink(i).m_jointType <= 2)
			    {
				    // 0, 1, 2 represent revolute, prismatic, and spherical joint types respectively. Skip the fixed joints.
				    double curPos = 0;
				    curPos = object->multibody->getJointPos(i);

				    startingPositions.push_back(curPos);
			    }
		    }
	    }

	    btScalar currentDiff = 1e30f;
	    b3AlignedObjectArray<double> endEffectorTargetWorldPositions;
	    b3AlignedObjectArray<double> endEffectorTargetWorldOrientations;
	    b3AlignedObjectArray<double> endEffectorCurrentWorldPositions;
	    b3AlignedObjectArray<double> jacobian_linear;
	    b3AlignedObjectArray<double> jacobian_angular;
	    btAlignedObjectArray<double> q_current;
	    btAlignedObjectArray<double> q_new;
	    btAlignedObjectArray<double> lower_limit;
	    btAlignedObjectArray<double> upper_limit;
	    btAlignedObjectArray<double> joint_range;
	    btAlignedObjectArray<double> rest_pose;
	    const int numDofs = object->multibody->getNumDofs();
	    int baseDofs = object->multibody->hasFixedBase() ? 0 : 6;
	    btInverseDynamics::vecx nu(numDofs + baseDofs), qdot(numDofs + baseDofs), q(numDofs + baseDofs), joint_force(numDofs + baseDofs);

	    endEffectorTargetWorldPositions.resize(0);
	    endEffectorTargetWorldPositions.reserve(numEndEffectorLinkIndices * 3);
	    endEffectorTargetWorldOrientations.resize(0);
	    endEffectorTargetWorldOrientations.reserve(numEndEffectorLinkIndices * 4);

	    bool validEndEffectorLinkIndices = true;

		// setarget position
	    for (int ne = 0; ne < numEndEffectorLinkIndices; ne++)
	    {
		    int endEffectorLinkIndex = endEffectorLinkIndices[ne];
		    validEndEffectorLinkIndices = validEndEffectorLinkIndices && (endEffectorLinkIndex < object->multibody->getNumLinks());

		    btVector3 targetPosWorld(targetPositions[ne][0],
		                             targetPositions[ne][1],
		                             targetPositions[ne][2]);

		    btQuaternion targetOrnWorld(targetOrientations[ne][0],
		                                targetOrientations[ne][1],
		                                targetOrientations[ne][2],
		                                targetOrientations[ne][3]);

		    btTransform targetBaseCoord;
		    btTransform targetWorld;
		    targetWorld.setOrigin(targetPosWorld);
		    targetWorld.setRotation(targetOrnWorld);
		    btTransform tr = object->multibody->getBaseWorldTransform();
		    targetBaseCoord = tr.inverse() * targetWorld;


		    btVector3DoubleData targetPosBaseCoord;
		    btQuaternionDoubleData targetOrnBaseCoord;
		    targetBaseCoord.getOrigin().serializeDouble(targetPosBaseCoord);
		    targetBaseCoord.getRotation().serializeDouble(targetOrnBaseCoord);

		    endEffectorTargetWorldPositions.push_back(targetPosBaseCoord.m_floats[0]);
		    endEffectorTargetWorldPositions.push_back(targetPosBaseCoord.m_floats[1]);
		    endEffectorTargetWorldPositions.push_back(targetPosBaseCoord.m_floats[2]);

		    endEffectorTargetWorldOrientations.push_back(targetOrnBaseCoord.m_floats[0]);
		    endEffectorTargetWorldOrientations.push_back(targetOrnBaseCoord.m_floats[1]);
		    endEffectorTargetWorldOrientations.push_back(targetOrnBaseCoord.m_floats[2]);
		    endEffectorTargetWorldOrientations.push_back(targetOrnBaseCoord.m_floats[3]);
	    }
		// IK iteration
	    for (int i = 0; i < numIterations && currentDiff > residualThreshold; i++)
	    {
		    if (ikHelperPtr && validEndEffectorLinkIndices)
		    {
			    jacobian_linear.resize(numEndEffectorLinkIndices * 3 * numDofs);
			    jacobian_angular.resize(numEndEffectorLinkIndices * 3 * numDofs);
			    int jacSize = 0;

                btInverseDynamics::MultiBodyTree* tree = 0;
                btInverseDynamics::btMultiBodyTreeCreator id_creator;
                if (-1 == id_creator.createFromBtMultiBody(object->multibody.get(), false))
                {
                }
                else
                {
                    tree = btInverseDynamics::CreateMultiBodyTree(id_creator);
                }

			    q_current.resize(numDofs);

			    if (tree && ((numDofs + baseDofs) == tree->numDoFs()))
			    {
				    btInverseDynamics::vec3 world_origin;
				    btInverseDynamics::mat33 world_rot;

				    jacSize = jacobian_linear.size();
				    // Set jacobian value

				    int DofIndex = 0;
				    for (int i = 0; i < object->multibody->getNumLinks(); ++i)
				    {
					    if (object->multibody->getLink(i).m_jointType >= 0 && object->multibody->getLink(i).m_jointType <= 2)
					    {
						    // 0, 1, 2 represent revolute, prismatic, and spherical joint types respectively. Skip the fixed joints.
						    double curPos = startingPositions[DofIndex];
						    q_current[DofIndex] = curPos;
						    q[DofIndex + baseDofs] = curPos;
						    qdot[DofIndex + baseDofs] = 0;
						    nu[DofIndex + baseDofs] = 0;
						    DofIndex++;
					    }
				    }  // Set the gravity to correspond to the world gravity
				    btInverseDynamics::vec3 id_grav(gravity);

				    {
					    if (-1 != tree->setGravityInWorldFrame(id_grav) &&
					        -1 != tree->calculateInverseDynamics(q, qdot, nu, &joint_force))
					    {
						    tree->calculateJacobians(q);
						    btInverseDynamics::mat3x jac_t(3, numDofs + baseDofs);
						    btInverseDynamics::mat3x jac_r(3, numDofs + baseDofs);
						    currentDiff = 0;

						    endEffectorCurrentWorldPositions.resize(0);
						    endEffectorCurrentWorldPositions.reserve(numEndEffectorLinkIndices * 3);

						    for (int ne = 0; ne < numEndEffectorLinkIndices; ne++)
						    {
							    int endEffectorLinkIndex2 = endEffectorLinkIndices[ne];

							    // Note that inverse dynamics uses zero-based indexing of bodies, not starting from -1 for the base link.
							    tree->getBodyJacobianTrans(endEffectorLinkIndex2 + 1, &jac_t);
							    tree->getBodyJacobianRot(endEffectorLinkIndex2 + 1, &jac_r);

							    //calculatePositionKinematics is already done inside calculateInverseDynamics

							    tree->getBodyOrigin(endEffectorLinkIndex2 + 1, &world_origin);
							    tree->getBodyTransform(endEffectorLinkIndex2 + 1, &world_rot);

							    for (int i = 0; i < 3; ++i)
							    {
								    for (int j = 0; j < numDofs; ++j)
								    {
									    jacobian_linear[(ne * 3 + i) * numDofs + j] = jac_t(i, (baseDofs + j));
									    jacobian_angular[(ne * 3 + i) * numDofs + j] = jac_r(i, (baseDofs + j));
								    }
							    }

							    endEffectorCurrentWorldPositions.push_back(world_origin[0]);
							    endEffectorCurrentWorldPositions.push_back(world_origin[1]);
							    endEffectorCurrentWorldPositions.push_back(world_origin[2]);

							    btInverseDynamics::vec3 targetPos(btVector3(endEffectorTargetWorldPositions[ne * 3 + 0],
							                                                endEffectorTargetWorldPositions[ne * 3 + 1],
							                                                endEffectorTargetWorldPositions[ne * 3 + 2]));
							    //diff
							    currentDiff = btMax(currentDiff, (world_origin - targetPos).length());
						    }
					    }
				    }

				    q_new.resize(numDofs);
					int IKMethod;
                    if (ikMethod == "JACOB_TRANS") {
                        IKMethod = IK2_JACOB_TRANS;
                    }
				    else if (ikMethod == "VEL_DLS_ORI_NULL")
				    {
					    //Nullspace task only works with DLS now. TODO: add nullspace task to SDLS.
					    IKMethod = IK2_VEL_DLS_WITH_ORIENTATION_NULLSPACE;
				    }
				    else if (ikMethod == "VEL_SDLS_ORI")
				    {
					    IKMethod = IK2_VEL_SDLS_WITH_ORIENTATION;
					}
					else if (ikMethod == "VEL_DLS_ORI")
					{
						IKMethod = IK2_VEL_DLS_WITH_ORIENTATION;
					}
				    else if (ikMethod == "VEL_DLS_NULL")
				    {
					    //Nullspace task only works with DLS now. TODO: add nullspace task to SDLS.
					    IKMethod = IK2_VEL_DLS_WITH_NULLSPACE;
				    }
				    else if (ikMethod == "VEL_SDLS")
				    {
					    IKMethod = IK2_VEL_SDLS;
					}
					else // VEL_DLS
					{
						IKMethod = IK2_VEL_DLS;
				    }

				    if (ikMethod == "VEL_DLS_ORI_NULL" || ikMethod == "VEL_DLS_NULL")
				    {
					    lower_limit.resize(numDofs);
					    upper_limit.resize(numDofs);
					    joint_range.resize(numDofs);
					    rest_pose.resize(numDofs);
					    for (int i = 0; i < numDofs; ++i) // TODO: use default data from multibody!
					    {
						    lower_limit[i] = object->multibody->getLink(i).m_jointLowerLimit;
						    upper_limit[i] = object->multibody->getLink(i).m_jointUpperLimit;
						    joint_range[i] = upper_limit[i] - lower_limit[i];
						    rest_pose[i] = startingPositions[i];
					    }
					    {
						    ikHelperPtr->computeNullspaceVel(numDofs, &q_current[0], &lower_limit[0], &upper_limit[0], &joint_range[0], &rest_pose[0]);
					    }
				    }

				    //btTransform endEffectorTransformWorld = bodyHandle->m_multiBody->getLink(endEffectorLinkIndex).m_cachedWorldTransform * bodyHandle->m_linkLocalInertialFrames[endEffectorLinkIndex].inverse();

				    btVector3DoubleData endEffectorWorldPosition;
				    btQuaternionDoubleData endEffectorWorldOrientation;

				    //get the position from the inverse dynamics (based on q) instead of endEffectorTransformWorld
				    btVector3 endEffectorPosWorldOrg = world_origin;
				    btQuaternion endEffectorOriWorldOrg;
				    world_rot.getRotation(endEffectorOriWorldOrg);

				    btTransform endEffectorBaseCoord;
				    endEffectorBaseCoord.setOrigin(endEffectorPosWorldOrg);
				    endEffectorBaseCoord.setRotation(endEffectorOriWorldOrg);

				    btQuaternion endEffectorOriBaseCoord = endEffectorBaseCoord.getRotation();

				    endEffectorBaseCoord.getOrigin().serializeDouble(endEffectorWorldPosition);
				    endEffectorBaseCoord.getRotation().serializeDouble(endEffectorWorldOrientation);

				    // Set joint damping coefficents. A small default
				    // damping constant is added to prevent singularity
				    // with pseudo inverse. The user can set joint damping
				    // coefficients differently for each joint. The larger
				    // the damping coefficient is, the less we rely on
				    // this joint to achieve the IK target.
				    btAlignedObjectArray<double> joint_damping;
				    joint_damping.resize(numDofs, 0.5);

				    for (int i = 0; i < numDofs; ++i)
				    {
					    joint_damping[i] = object->multibody->getLink(i).m_jointDamping;
				    }
				    ikHelperPtr->setDampingCoeff(numDofs, &joint_damping[0]);

				    double targetDampCoeff[6] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
				    bool performedIK = false;
				    if (numEndEffectorLinkIndices == 1)
				    {
					    ikHelperPtr->computeIK(&endEffectorTargetWorldPositions[0],
					                           &endEffectorTargetWorldOrientations[0],
					                           endEffectorWorldPosition.m_floats, endEffectorWorldOrientation.m_floats,
					                           &q_current[0],
					                           numDofs, endEffectorLinkIndices[0],
					                           &q_new[0], IKMethod, &jacobian_linear[0], &jacobian_angular[0], jacSize * 2, targetDampCoeff);
					    performedIK = true;
				    }
				    else
				    {
					    if (numEndEffectorLinkIndices > 1)
					    {
						    ikHelperPtr->computeIK2(&endEffectorTargetWorldPositions[0],
						                            &endEffectorCurrentWorldPositions[0],
						                            numEndEffectorLinkIndices,
						    //endEffectorWorldOrientation.m_floats,
						                            &q_current[0],
						                            numDofs,
						                            &q_new[0], IKMethod, &jacobian_linear[0], targetDampCoeff);
						    performedIK = true;
					    }
				    }
				    if (performedIK)
				    {
					    for (int i = 0; i < numDofs; i++)
					    {
						    startingPositions[i] = q_new[i];
					    }
				    }
			    }
		    }
	    }
        auto outputPoses = create_ListObject();
        outputPoses->clear();
        for (size_t i = 0; i < startingPositions.size(); i++){
            auto p = std::make_shared<zeno::NumericObject>(float(startingPositions[i]));
            outputPoses->push_back(p);
        }
	    set_output("poses", std::move(outputPoses));
    }
};

ZENDEFNODE(BulletCalcInverseKinematics, {
    {{gParamType_Unknown, "object"},
        {gParamType_Vec3f, "gravity"},
        {gParamType_List, "endEffectorLinkIndices"},
        {gParamType_List, "targetPositions"},
        {gParamType_List, "targetOrientations"},
        {gParamType_Int, "numIterations", "20"},
        {gParamType_Float, "residualThreshold", "0.0001"}},
    {{gParamType_List, "poses"}},
	{{"enum VEL_DLS_ORI_NULL VEL_SDLS_ORI VEL_DLS_ORI VEL_DLS_NULL VEL_SDLS VEL_DLS JACOB_TRANS", "IKMethod", "VEL_DLS_ORI_NULL"}},
	{"Bullet"},
});


struct BulletMultiBodyMakeJointMotor : zeno::INode {
    virtual void apply() override {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
        auto linkIndex = get_input2_int("linkIndex");
        int linkDof = 0;
        if (has_input("linkDof")){
            linkDof = get_input2_int("linkDof");
        }
        auto desiredVelocity = btScalar(get_input2_float("desiredVelocity"));
        auto maxMotorImpulse = btScalar(get_input2_float("maxMotorImpulse"));

        auto jointMotor = std::make_unique<BulletMultiBodyJointMotor>(object->multibody.get(), linkIndex, linkDof, desiredVelocity, maxMotorImpulse);
        world->dynamicsWorld->addMultiBodyConstraint(jointMotor->jointMotor.get());
        jointMotor->jointMotor->finalizeMultiDof();
        set_output("object", std::move(object));
        set_output("world", std::move(world));
    }
};

ZENDEFNODE(BulletMultiBodyMakeJointMotor, {
    {{gParamType_Unknown, "object"}, {gParamType_Int, "linkIndex"}, {gParamType_Int, "linkDof", "0"}, {gParamType_Float, "desiredVelocity", "0"}, {gParamType_Float, "maxMotorImpulse", "10.0"}},
    {{gParamType_Unknown, "object"}, {gParamType_Unknown, "world"}},
    {},
    {"Bullet"}
});


struct BulletMultiBodyGetJointTorque : zeno::INode {
    virtual void apply() override {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        auto link_id = get_input2_int("jointIndex");
        btScalar torque;

        torque = object->multibody->getJointTorque(link_id);
        // out_torque = vec1f(other_to_vec<1>(torque));

        auto out_torque = std::make_shared<zeno::NumericObject>(torque);
        set_output("joint_torque", std::move(out_torque));
    }
};

ZENDEFNODE(BulletMultiBodyGetJointTorque, {
    {{gParamType_Unknown, "object"}, {gParamType_Int, "jointIndex"}},
    {{gParamType_Unknown, "torque"}},
    {},
    {"Bullet"}
});

struct BulletMultiBodyGetLinkForce : zeno::INode {
    virtual void apply() override {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        auto link_id = get_input2_int("linkIndex");
        btVector3 force;
        auto force_out = zeno::IObject::make<zeno::NumericObject>();
        force = object->multibody->getLinkForce(link_id);
        force_out->set<zeno::vec3f>(zeno::vec3f(force.x(), force.y(), force.z()));
        set_output("force", std::move(force_out));
    }
};

ZENDEFNODE(BulletMultiBodyGetLinkForce, {
                                              {{gParamType_Unknown, "object"}, {gParamType_Int, "linkIndex"}},
                                              {{gParamType_Vec3f,"force"}},
                                              {},
                                              {"Bullet"}
                                          });

struct BulletMultiBodyGetLinkTorque : zeno::INode {
    virtual void apply() override {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        auto link_id = get_input2_int("linkIndex");
        btVector3 torque;
        auto torque_out = zeno::IObject::make<zeno::NumericObject>();
        torque = object->multibody->getLinkTorque(link_id);
        torque_out->set<zeno::vec3f>(zeno::vec3f(torque.x(), torque.y(), torque.z()));
        set_output("torque", std::move(torque_out));
    }
};

ZENDEFNODE(BulletMultiBodyGetLinkTorque, {
                                            {{gParamType_Unknown, "object"}, {gParamType_Int, "linkIndex"}},
                                            {{gParamType_Vec3f,"torque"}},
                                            {},
                                            {"Bullet"}
                                        });

struct BulletMultiBodyGetJointVelPos : zeno::INode {
    virtual void apply() override {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        auto link_id = get_input2_int("linkIndex");
        btScalar vel;
        btScalar pos;

        vel = object->multibody->getJointVel(link_id);
        pos = object->multibody->getJointPos(link_id);
        // out_torque = vec1f(other_to_vec<1>(torque));

        auto vel_ = std::make_shared<zeno::NumericObject>(vel);
        auto pos_ = std::make_shared<zeno::NumericObject>(pos);
        set_output("vel", std::move(vel_));
        set_output("pos", std::move(pos_));
    }
};

ZENDEFNODE(BulletMultiBodyGetJointVelPos, {
                                              {{gParamType_Unknown, "object"}, {gParamType_Int, "linkIndex"}},
                                              {{gParamType_Float,"vel"}, {gParamType_Float,"pos"}},
                                              {},
                                              {"Bullet"}
                                          });

struct BulletMultiBodyGetBaseTransform : zeno::INode {
    virtual void apply() {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        auto trans = std::make_unique<BulletTransform>();
        trans->trans = object->multibody->getBaseWorldTransform();
        set_output("trans", std::move(trans));
    }
};

 ZENDEFNODE(BulletMultiBodyGetBaseTransform, {
     {{gParamType_Unknown, "object"}},
     {{gParamType_Unknown, "trans"}},
     {},
     {"Bullet"},
 });

 struct BulletMultiBodyGetBaseVelocity : zeno::INode {
     virtual void apply() {
         auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
         auto vel = zeno::IObject::make<zeno::NumericObject>();
         btVector3 vel_;
         vel_ = object->multibody->getBaseVel();
         vel->set<zeno::vec3f>(zeno::vec3f(vel_.x(), vel_.y(), vel_.z()));
         set_output("vel", std::move(vel));
     }
 };

 ZENDEFNODE(BulletMultiBodyGetBaseVelocity, {
                                                 {{gParamType_Unknown, "object"}},
                                                 {{gParamType_Vec3f,"vel"}},
                                                 {},
                                                 {"Bullet"},
                                             });

struct BulletMultiBodyClearJointStates : zeno::INode {
    virtual void apply() {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        object->multibody->clearVelocities();
        object->multibody->clearForcesAndTorques();
//        object->multibody->clearConstraintForces();
//        Not sure if clear constraint forces is necessary
        set_output("object", std::move(object));
    }
};

ZENDEFNODE(BulletMultiBodyClearJointStates, {
                                               {{gParamType_Unknown, "object"}},
                                               {{gParamType_Unknown, "object"}},
                                               {},
                                               {"Bullet"},
                                           });

struct BulletMultiBodyApplyExternalTorque : zeno::INode {
    virtual void apply() {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        //        auto linkIndex = zeno::IObject::make<zeno::NumericObject>();
        auto linkIndex = get_input2_int("linkIndex");
        auto torque = zeno::vec_to_other<btVector3>(
            toVec3f(get_input2_vec3f("torque")));
        auto pos = zeno::vec_to_other<btVector3>(
            toVec3f(get_input2_vec3f("pos")));
        auto isLinkFrame = get_input2_bool("isLinkFrame");

        // LinkIndex = -1 for applying force to the base link
        if (linkIndex == -1) {
            auto torqueWorld =
                isLinkFrame
                    ? object->multibody->getBaseWorldTransform().getBasis() *
                          torque
                    : torque;
            object->multibody->addBaseTorque(torque);
        } else {
            auto torqueWorld =
                isLinkFrame ? object->multibody->getLink(linkIndex)
                                      .m_cachedWorldTransform.getBasis() *
                                  torque
                            : torque;
            object->multibody->addLinkTorque(linkIndex, torque);
        }

        set_output("object", std::move(object));
    }
};

ZENDEFNODE(BulletMultiBodyApplyExternalTorque,
           {
               {{gParamType_Unknown, "object"}, {gParamType_Int, "linkIndex"}, {gParamType_Vec3f, "torque"}},
               {{gParamType_Unknown, "object"}},
               {},
               {"Bullet"},
           });

struct BulletMultiBodyApplyExternalForce : zeno::INode {
    virtual void apply() {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        //        auto linkIndex = zeno::IObject::make<zeno::NumericObject>();
        auto linkIndex = get_input2_int("linkIndex");
        auto force = zeno::vec_to_other<btVector3>(
            toVec3f(get_input2_vec3f("force")));
        auto pos = zeno::vec_to_other<btVector3>(
            toVec3f(get_input2_vec3f("pos")));
        auto isLinkFrame = get_input2_bool("isLinkFrame");

        // LinkIndex = -1 for applying force to the base link
        if (linkIndex == -1) {
            auto forceWorld =
                isLinkFrame
                    ? object->multibody->getBaseWorldTransform().getBasis() *
                          force
                    : force;
            auto relPosWorld =
                isLinkFrame
                    ? object->multibody->getBaseWorldTransform().getBasis() *
                          pos
                    : pos - object->multibody->getBaseWorldTransform()
                                .getOrigin();
            object->multibody->addBaseForce(forceWorld);
            object->multibody->addBaseTorque((relPosWorld.cross(forceWorld)));
        } else {
            auto forceWorld = isLinkFrame
                                  ? object->multibody->getLink(linkIndex)
                                            .m_cachedWorldTransform.getBasis() *
                                        force
                                  : force;
            auto relPosWorld =
                isLinkFrame ? object->multibody->getLink(linkIndex)
                                      .m_cachedWorldTransform.getBasis() *
                                  pos
                            : pos - object->multibody->getLink(linkIndex)
                                        .m_cachedWorldTransform.getOrigin();
            object->multibody->addLinkForce(linkIndex, forceWorld);
            object->multibody->addLinkTorque(linkIndex,
                                             relPosWorld.cross(forceWorld));
        }

        set_output("object", std::move(object));
    }
};

ZENDEFNODE(BulletMultiBodyApplyExternalForce,
           {
               {{gParamType_Unknown, "object"}, {gParamType_Int, "linkIndex"}, {gParamType_Vec3f, "force"}},
               {{gParamType_Unknown, "object"}},
               {},
               {"Bullet"},
           });

struct BulletResetSimulation : zeno::INode {
    virtual void apply() {
        auto world = safe_uniqueptr_cast<BulletWorldProxy>(clone_input("world"));
        world->collisionWorld.reset();
        set_output("world", std::move(world));
    }
};

ZENDEFNODE(BulletResetSimulation, {
                                      {{gParamType_Unknown, "world"}},
                                      {{gParamType_Unknown, "world"}},
                                      {},
                                      {"Bullet"},
                                  });

struct BulletMultiBodyResetSimulation : zeno::INode {
    virtual void apply() {
        auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
        world->dynamicsWorld.reset();
        set_output("world", std::move(world));
    }
};

ZENDEFNODE(BulletMultiBodyResetSimulation, {
                                               {{gParamType_Unknown, "world"}},
                                               {{gParamType_Unknown, "world"}},
                                               {},
                                               {"Bullet"},
                                           });

struct BulletMultiBodyCalculateJacobian : zeno::INode {
    virtual void apply() {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        auto linkIndex = get_input2_int("linkIndex");
        auto localPosition = get_input2<zeno::vec3f>("localPos");

        std::vector<float> jointPositionsQ;
        {
            auto numericObjs = get_input_ListObject("jointPositionsQ")
                                   ->get<std::decay_t<NumericObject>>();
            for (auto &&no : numericObjs)
                jointPositionsQ.push_back(no->get<float>());
        }

        std::vector<float> jointVelocitiesQd;
        {
            auto numericObjs = get_input_ListObject("jointVelocitiesQd")
                                   ->get<std::decay_t<NumericObject>>();
            for (auto &&no : numericObjs)
                jointVelocitiesQd.push_back(no->get<float>());
        }

        std::vector<float> jointAccelerations;
        {
            auto numericObjs = get_input_ListObject("jointAccelerations")
                                   ->get<std::decay_t<NumericObject>>();
            for (auto &&no : numericObjs)
                jointAccelerations.push_back(no->get<float>());
        }

        btVector3 id_grav(0, -9.81, 0);
        if (has_input("gravity")) {
            id_grav = zeno::vec_to_other<btVector3>(
                toVec3f(get_input2_vec3f("gravity")));
        }

        b3AlignedObjectArray<double> jacobian_linear;
        b3AlignedObjectArray<double> jacobian_angular;

        btInverseDynamics::MultiBodyTree *tree = 0;
        btInverseDynamics::btMultiBodyTreeCreator id_creator;
        if (-1 == id_creator.createFromBtMultiBody(
                      object->multibody.get(), false)) {
        } else {
            tree = btInverseDynamics::CreateMultiBodyTree(id_creator);
        }

        if(tree){
            int baseDofs = object->multibody->hasFixedBase() ? 0 : 6;
            const int numDofs = object->multibody->getNumDofs();

            btInverseDynamics::vecx q(numDofs + baseDofs);
            btInverseDynamics::vecx qdot(numDofs + baseDofs);
            btInverseDynamics::vecx nu(numDofs + baseDofs);
            btInverseDynamics::vecx joint_force(numDofs + baseDofs);

            for (int i = 0; i < numDofs; i++) {
                q[i + baseDofs] = jointPositionsQ[i]; //jointPositionsQ?
                qdot[i + baseDofs] = jointVelocitiesQd[i];
                nu[i + baseDofs] = jointAccelerations[i];
            }

            // Set the gravity to correspond to the world gravity
//            btInverseDynamics::vec3 id_grav(m_data->m_dynamicsWorld->getGravity());
            if (-1 != tree->setGravityInWorldFrame(id_grav) &&
                -1 != tree->calculateInverseDynamics(q, qdot, nu, &joint_force))
            {
                auto m_dofCount = numDofs + baseDofs;
                // Set jacobian value
                tree->calculateJacobians(q);
                btInverseDynamics::mat3x jac_t(3, numDofs + baseDofs);
                btInverseDynamics::mat3x jac_r(3, numDofs + baseDofs);

                // Note that inverse dynamics uses zero-based indexing of bodies, not starting from -1 for the base link.
                tree->getBodyJacobianTrans(linkIndex + 1, &jac_t);
                tree->getBodyJacobianRot(linkIndex + 1, &jac_r);
                // Update the translational jacobian based on the desired local point.
                // v_pt = v_frame + w x pt
                // v_pt = J_t * qd + (J_r * qd) x pt
                // v_pt = J_t * qd - pt x (J_r * qd)
                // v_pt = J_t * qd - pt_x * J_r * qd)
                // v_pt = (J_t - pt_x * J_r) * qd
                // J_t_new = J_t - pt_x * J_r
                btInverseDynamics::vec3 localPosition;
                for (int i = 0; i < 3; ++i)
                {
                    localPosition(i) = localPosition[i];
                }
                // Only calculate if the localPosition is non-zero.
                if (btInverseDynamics::maxAbs(localPosition) > 0.0)
                {
                    // Write the localPosition into world coordinates.
                    btInverseDynamics::mat33 world_rotation_body;
                    tree->getBodyTransform(linkIndex + 1, &world_rotation_body);
                    localPosition = world_rotation_body * localPosition;
                    // Correct the translational jacobian.
                    btInverseDynamics::mat33 skewCrossProduct;
                    btInverseDynamics::skew(localPosition, &skewCrossProduct);
                    btInverseDynamics::mat3x jac_l(3, numDofs + baseDofs);
                    btInverseDynamics::mul(skewCrossProduct, jac_r, &jac_l);
                    btInverseDynamics::mat3x jac_t_new(3, numDofs + baseDofs);
                    btInverseDynamics::sub(jac_t, jac_l, &jac_t_new);
                    jac_t = jac_t_new;
                }
                // Fill in the result into the shared memory.
                for (int i = 0; i < 3; ++i)
                {
                    for (int j = 0; j < (numDofs + baseDofs); ++j)
                    {
                        int element = (numDofs + baseDofs) * i + j;
                        jacobian_linear[element] = jac_t(i, j);
                        jacobian_angular[element] = jac_r(i, j);
                    }
                }
;
            }
        }
struct BulletPerformCollisionDetection : zeno::INode {
    virtual void apply() {
        auto worldType = zsString2Std(get_input2_string("worldType"));
        if (worldType == "multi") {
            auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
            world->dynamicsWorld->performDiscreteCollisionDetection();
            //            world->dynamicsWorld->contac
            set_output("world", std::move(world));
        } else {
            auto world = safe_uniqueptr_cast<BulletWorldProxy>(clone_input("world"));
            world->dynamicsWorld->performDiscreteCollisionDetection();
            set_output("world", std::move(world));
        }
    }
};

ZENDEFNODE(BulletPerformCollisionDetection,
           {
               {{gParamType_Unknown, "world"}},
               {},
               {{"enum multi rigid", "worldType", "multi"}},
               {"Bullet"},
           });

struct BulletMultiBodyAddLinkForce : zeno::INode {
    virtual void apply() {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        //        auto linkIndex = zeno::IObject::make<zeno::NumericObject>();
        auto linkIndex = get_input2_int("linkIndex");
        auto force = zeno::vec_to_other<btVector3>(
            toVec3f(get_input2_vec3f("force")));
        object->multibody->addLinkForce(linkIndex, force);
        set_output("object", std::move(object));
    }
};

ZENDEFNODE(BulletMultiBodyAddLinkForce, {
                                            {{gParamType_Unknown, "object"}, {gParamType_Int, "linkIndex"}, {gParamType_Vec3f, "force"}},
                                            {{gParamType_Unknown, "object"}},
                                            {},
                                            {"Bullet"},
                                        });



        auto output_jac_linear = create_ListObject();
        output_jac_linear->clear();
        for (size_t i = 0; i < jacobian_linear.size(); i++) {
            auto p = std::make_shared<zeno::NumericObject>(
                float(jacobian_linear[i]));
            output_jac_linear->push_back(p);
        }
        auto output_jac_angular = create_ListObject();
        output_jac_angular->clear();
        for (size_t i = 0; i < jacobian_angular.size(); i++) {
            auto p = std::make_shared<zeno::NumericObject>(
                float(jacobian_angular[i]));
            output_jac_angular->push_back(p);
        }
        set_output("object", std::move(object));
        set_output("jacobian_linear", std::move(output_jac_linear));
        set_output("jacobian_angular", std::move(output_jac_angular));
    }
};

ZENDEFNODE(BulletMultiBodyCalculateJacobian, {
                                               {{gParamType_Unknown, "object"}, 
                                                {gParamType_Int, "linkIndex"},
                                                {gParamType_Vec3f, "localPos"},
                                                {gParamType_List, "jointPositionsQ"},
                                                {gParamType_List, "jointVelocitiesQd"},
                                                {gParamType_List, "jointAccelerations"},
                                                {gParamType_Vec3f, "gravity"}},
                                               {{gParamType_Unknown, "object"},
                                                {gParamType_List, "jacobian_linear"},
                                                {gParamType_List, "jacobian_angular"}},
                                               {},
                                               {"Bullet"},
                                           });



//struct BulletMultiBodyWorldChangeDynamics : zeno::INode {
//    virtual void apply() override {
//        auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
//        auto
//
//            world->dynamicsWorld->set
//
//                set_output("world", std::move(world));
//    }
//};

struct BulletMultiBodyCalculateMassMatrix : zeno::INode {
    virtual void apply() {
        auto object = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        std::vector<float> jointPositionsQ;
        {
            auto numericObjs = get_input_ListObject("jointPositionsQ")
                                   ->get<std::decay_t<NumericObject>>();
            for (auto &&no : numericObjs)
                jointPositionsQ.push_back(no->get<float>());
        }

        btInverseDynamics::MultiBodyTree *tree = 0;
        btInverseDynamics::btMultiBodyTreeCreator id_creator;
        if (-1 ==
            id_creator.createFromBtMultiBody(object->multibody.get(), false)) {
        } else {
            tree = btInverseDynamics::CreateMultiBodyTree(id_creator);
        }

        if (tree) {
            int baseDofs = object->multibody->hasFixedBase() ? 0 : 6;
            const int numDofs = object->multibody->getNumDofs();
            const int totDofs = numDofs + baseDofs;
            btInverseDynamics::vecx q(totDofs);
            btInverseDynamics::matxx massMatrix(totDofs, totDofs);
            for (int i = 0; i < numDofs; i++) {
                q[i + baseDofs] = jointPositionsQ[i];
            }
            auto output_mass_matrix = create_ListObject();
            if (-1 != tree->calculateMassMatrix(q, &massMatrix)) {
                //                serverCmd.m_massMatrixResultArgs.m_dofCount = totDofs;
                // Fill in the result into the shared memory.
                //                double* sharedBuf = (double*)bufferServerToClient;
                int sizeInBytes = totDofs * totDofs * sizeof(double);

                for (int i = 0; i < (totDofs); ++i) {
                    for (int j = 0; j < (totDofs); ++j) {
                        int element = (totDofs)*i + j;
                        auto p = std::make_shared<zeno::NumericObject>(
                            float(massMatrix(i, j)));
                        output_mass_matrix->push_back(p);
                    }
                }
            }
            set_output("mass_matrix", std::move(output_mass_matrix));
            set_output("object", std::move(object));
        }
    }
};

ZENDEFNODE(BulletMultiBodyCalculateMassMatrix, {
                                                 {{gParamType_Unknown, "object"}, {gParamType_List, "jointPositionsQ"}},
                                                 {{gParamType_Unknown, "object"}, {gParamType_List, "mass_matrix"}},
                                                 {},
                                                 {"Bullet"},
                                             });

struct BulletMultiBodyGetNumBodies : zeno::INode {
    virtual void apply() {
        auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
        int num_ = world->dynamicsWorld->getNumMultibodies();
        auto num = std::make_shared<zeno::NumericObject>(num_);
        set_output("numBodies", std::move(num));
    }
};

ZENDEFNODE(BulletMultiBodyGetNumBodies, {
                                                   {{gParamType_Unknown, "world"}},
                                                   {{gParamType_Int,"numBodies"}},
                                                   {},
                                                   {"Bullet"},
                                               });

struct BulletMultiBodyGetBodyId : zeno::INode {
    virtual void apply() {
        auto obj = safe_uniqueptr_cast<BulletMultiBodyObject>(move_input("object"));
        auto id_ = obj->multibody->getUserIndex();
        auto id = std::make_shared<zeno::NumericObject>(id_);
        set_output("id", std::move(id));
    }
};
ZENDEFNODE(BulletMultiBodyGetBodyId, {
                                            {{gParamType_Unknown, "object"}},
                                            {{gParamType_Int,"id"}},
                                            {},
                                            {"Bullet"},
                                        });

struct BulletMultiBodyRemoveBody : zeno::INode {
    virtual void apply() {
        auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
        auto id = get_input2_int("id");
        auto body = world->dynamicsWorld->getMultiBody(id);
        world->dynamicsWorld->removeMultiBody(body);
        set_output("id", std::move(world));
    }
};
ZENDEFNODE(BulletMultiBodyRemoveBody, {
                                         {{gParamType_Unknown, "world"}, {gParamType_Int,"id"}},
                                         {{gParamType_Unknown, "world"}},
                                         {},
                                         {"Bullet"},
                                     });


struct BulletMultiBodyGetContactPoints : zeno::INode {
    virtual void apply() {
        auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
        int numManifolds = world->dispatcher->getNumManifolds();
        auto contactList = create_ListObject();
        for(int i=0; i<numManifolds; ++i){
            btPersistentManifold* contactManifold = world->dispatcher->getManifoldByIndexInternal(i);
            int numContacts = contactManifold->getNumContacts();
            auto contactPairsList = create_ListObject();
            for (int j=0; j<numContacts; j++){
                btManifoldPoint& pt = contactManifold->getContactPoint(j);

                btVector3 ptA = pt.getPositionWorldOnA();
                btVector3 ptB = pt.getPositionWorldOnB();

                auto pA = std::make_shared<zeno::NumericObject>();
                auto pB = std::make_shared<zeno::NumericObject>();

                pA->set<zeno::vec3f>(zeno::vec3f(ptA.x(), ptA.y(), ptA.z()));
                pB->set<zeno::vec3f>(zeno::vec3f(ptB.x(), ptB.y(), ptB.z()));

                contactPairsList->push_back(pA);
                contactPairsList->push_back(pB);
            }
            contactList->push_back(contactPairsList);
        }
        set_output("world", std::move(world));
        set_output("contactPointsList", std::move(contactList));
    }
};

ZENDEFNODE(BulletMultiBodyGetContactPoints, {
                                                {{gParamType_Unknown, "world"}},
                                                {{gParamType_List, "contactPointsList"}},
                                                {},
                                                {"Bullet"},
                                            });

struct BulletGetAABB : zeno::INode {
    virtual void apply() {
        auto object= safe_uniqueptr_cast<BulletObjectProxy>(clone_input("object"));
        auto transform = object->body->getWorldTransform();
        btVector3 *aabbMin{};
        btVector3 *aabbMax{};
        object->colShape->shape->getAabb(transform, *aabbMin, *aabbMax);

        auto aabbMin_out = std::make_shared<zeno::NumericObject>();
        aabbMin_out->set<zeno::vec3f>(zeno::vec3f(aabbMin->x(), aabbMin->y(), aabbMin->z()));

        auto aabbMax_out = std::make_unique<zeno::NumericObject>();
        aabbMax_out->set<zeno::vec3f>(zeno::vec3f(aabbMax->x(), aabbMax->y(), aabbMax->z()));

        set_output("aabbMin", std::move(aabbMin_out));
        set_output("aabbMax", std::move(aabbMax_out));
    }
};

ZENDEFNODE(BulletGetAABB, {
                                                {{gParamType_Unknown, "object"}},
                                                {{gParamType_Vec3f,"aabbMin"}, {gParamType_Vec3f,"aabbMax"}},
                                                {},
                                                {"Bullet"},
                                            });

struct MyBroadphaseCallback : public btBroadphaseAabbCallback
{
    b3AlignedObjectArray<int> m_bodyUniqueIds;
    b3AlignedObjectArray<int> m_links;

    MyBroadphaseCallback()
    {
    }
    virtual ~MyBroadphaseCallback()
    {
    }
    void clear()
    {
        m_bodyUniqueIds.clear();
        m_links.clear();
    }
    virtual bool process(const btBroadphaseProxy* proxy)
    {
        btCollisionObject* colObj = (btCollisionObject*)proxy->m_clientObject;
        btMultiBodyLinkCollider* mbl = btMultiBodyLinkCollider::upcast(colObj);
        if (mbl)
        {
            int bodyUniqueId = mbl->m_multiBody->getUserIndex2();
            m_bodyUniqueIds.push_back(bodyUniqueId);
            m_links.push_back(mbl->m_link);
            return true;
        }
        int bodyUniqueId = colObj->getUserIndex2();
        if (bodyUniqueId >= 0)
        {
            m_bodyUniqueIds.push_back(bodyUniqueId);
            //it is not a multibody, so use -1 otherwise
            m_links.push_back(-1);
        }
        return true;
    }
};

//struct BulletMultiBodyGetAABB : zeno::INode {
//    virtual void apply(){
//        auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
//        auto id = get_input<zeno::NumericObject>("id");
//
//        btVector3 *aabbMin;
//        btVector3 *aabbMax;
//        btBroadphaseProxy* proxy = world->dynamicsWorld->getBroadphase()->createProxy(*aabbMin, *aabbMax);
//        world->dynamicsWorld->getBroadphase()->getAabb(proxy, );
//    }
//};
//
//struct BulletMultiBodyGetOverLappingObjects : zeno::INode {
//  virtual void apply(){
//      auto world = safe_uniqueptr_cast<BulletMultiBodyWorld>(move_input("world"));
//      auto aabbMin = get_input2<btVector3>("aabbMin");
//      auto aabbMax = get_input2<btVector3>("aabbMax");
//
//      world->dynamicsWorld->getBroadphase()->aabbTest(aabbMin, aabbMax, world->dynamicsWorld->getBroadphase()->);
//  }
//};
//
//struct BulletMultiBodyChangeDynamics : zeno::INode {
//
//};

#endif

}; // namespace
