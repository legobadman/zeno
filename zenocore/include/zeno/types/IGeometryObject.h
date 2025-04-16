#pragma once

#ifndef __IGEOMETRY_OBJECT_H__
#define __IGEOMETRY_OBJECT_H__

#include <set>
#include <zeno/core/coredata.h>
#include <zeno/core/common.h>
#include <zeno/core/IObject.h>
#include <zeno/utils/api.h>

namespace zeno
{
    struct GeometryObject;  //private data.
    struct PrimitiveObject;

    using namespace zeno::reflect;

    struct ZENO_API GeometryObject_Adapter : IObject
    {
        zany clone() const override;
        void Delete() override;

        void inheritAttributes(
            GeometryObject_Adapter* rhs,
            int vtx_offset,
            int pt_offset,
            std::set<zeno::String> pt_nocopy,    //TODO abi problem
            int face_offset,
            std::set<zeno::String> face_nocopy
        );

        Vector<Vec3f> points_pos();
        Vector<Vec3i> tri_indice() const;
        Vector<int> edge_list() const;

        bool is_base_triangle() const;
        bool is_Line() const;
        int get_group_count(GeoAttrGroup grp) const;
        GeoAttrType get_attr_type(GeoAttrGroup grp, const zeno::String& name);
        zeno::Vector<zeno::String> get_attr_names(GeoAttrGroup grp);
        //void geomTriangulate(zeno::TriangulateInfo& info);

        //��������
        int create_attr(GeoAttrGroup grp, const zeno::String& attr_name, const Any& defl);
        int create_face_attr(const zeno::String& attr_name, const Any& defl);
        int create_point_attr(const zeno::String& attr_name, const Any& defl);
        int create_vertex_attr(const zeno::String& attr_name, const Any& defl);
        int create_geometry_attr(const zeno::String& attr_name, const Any& defl);

        //��������
        int set_attr(GeoAttrGroup grp, const zeno::String& name, const Any& val);
        int set_vertex_attr(const zeno::String& attr_name, const Any& defl);
        int set_point_attr(const zeno::String& attr_name, const Any& defl);
        int set_face_attr(const zeno::String& attr_name, const Any& defl);
        int set_geometry_attr(const zeno::String& attr_name, const Any& defl);

        /* ��������Ƿ���� */
        bool has_attr(GeoAttrGroup grp, const zeno::String& name);
        bool has_vertex_attr(const zeno::String& name) const;
        bool has_point_attr(const zeno::String& name) const;
        bool has_face_attr(const zeno::String& name) const;
        bool has_geometry_attr(const zeno::String& name) const;

        //ɾ������
        int delete_attr(GeoAttrGroup grp, const zeno::String& attr_name);
        int delete_vertex_attr(const zeno::String& attr_name);
        int delete_point_attr(const zeno::String& attr_name);
        int delete_face_attr(const zeno::String& attr_name);
        int delete_geometry_attr(const zeno::String& attr_name);

        /* ���Ԫ�� */
        int add_vertex(int face_id, int point_id);
        int add_point(Vec3f pos);
        int add_face(const zeno::Vector<int>& points, bool bClose = true);
        void set_face(int idx, const zeno::Vector<int>& points, bool bClose = true);

        /* �Ƴ�Ԫ����� */
        bool remove_faces(const std::set<int>& faces, bool includePoints);
        bool remove_point(int ptnum);
        bool remove_vertex(int face_id, int vert_id);

        /* ����Ԫ�ظ��� */
        int npoints() const;
        int nfaces() const;
        int nvertices() const;
        int nvertices(int face_id) const;
        int nattributes(GeoAttrGroup grp) const;

        /* ����� */
        zeno::Vector<int> point_faces(int point_id);
        int point_vertex(int point_id);
        zeno::Vector<int> point_vertices(int point_id);

        /* ����� */
        int face_point(int face_id, int vert_id) const;
        zeno::Vector<int> face_points(int face_id);
        int face_vertex(int face_id, int vert_id);
        int face_vertex_count(int face_id);
        zeno::Vector<int> face_vertices(int face_id);
        zeno::Vec3f face_normal(int face_id);

        /* Vertex��� */
        int vertex_index(int face_id, int vertex_id);
        int vertex_next(int linear_vertex_id);
        int vertex_prev(int linear_vertex_id);
        int vertex_point(int linear_vertex_id);
        int vertex_face(int linear_vertex_id);
        int vertex_face_index(int linear_vertex_id);
        std::tuple<int, int, int> vertex_info(int linear_vertex_id);

        GeometryObject* m_impl;     //TODO: unique_ptr with abi compatible
    };

    //û�зŵ����棬����Ϊ�û�Ҫֱ��include��ǰ�ļ����ɴ�ֱ�������ﹹ��
    ZENO_API zeno::SharedPtr<GeometryObject_Adapter> create_GeometryObject();
    ZENO_API zeno::SharedPtr<GeometryObject_Adapter> create_GeometryObject(bool bTriangle, int nPoints, int nFaces, bool bInitFaces = false);
    ZENO_API zeno::SharedPtr<GeometryObject_Adapter> create_GeometryObject(PrimitiveObject* prim);
    ZENO_API zeno::SharedPtr<GeometryObject_Adapter> clone_GeometryObject(zeno::SharedPtr<GeometryObject_Adapter> pGeom);
}

#endif