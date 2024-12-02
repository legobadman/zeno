// ===============================================
// =============== 页面的逻辑控制器 ===============
// ===============================================


import QtQuick 2.15


Item {

    // ========================= 【列表】 =========================

    /* 所有页面的静态信息
            url:   页面qml文件路径
            title: 页面标题
            intro: 页面简介
            comp:  页面组件（qml文件）
    */
    property var infoList: [
        {
            url: "qrc:/zenographview.qml",
            title: "main",
            intro: "主图"
        },
    ]

    /* 存放当前已打开的页面
            obj:        页面组件对象
            info:       页面信息（infoList中对应项的引用）
            infoIndex:  页面信息下标（infoList中对应项的引用）
    */
    property var pageList: []

    // ========================= 【增删改查】 =========================

    // 初始化： 将 infoList 的 url 转换为可实例化的组件类 comp
    function initListUrl() {
        for(let i=infoList.length-1; i>=0; i--){
            const comp = Qt.createComponent(infoList[i].url)
            if (comp.status === Component.Ready) { // 加载成功
                infoList[i].comp = comp
            } else{ // 加载失败
                infoList[i].comp = undefined
                if (comp.status === Component.Error) {  // 加载失败，提取错误信息
                    let str = comp.errorString()
                    const last = str.lastIndexOf(":")
                    if(last < 0) last = -1
                    str = str.substring(last+1).replace("\n","")
                    console.error(`【Error】加载页面文件失败：【${infoList[i].url}】${str}`)
                }
                else{
                    console.error(`【Error】加载页面文件异常：【${infoList[i].url}】`)
                }
            }
        }
        console.log("infoList comp 初始化完成")
    }

    function addPage2(index, owner, graphM) {
        const comp = Qt.createComponent("qrc:/zenographview.qml")
        var args = {z: 0, visible: true, graphModel: graphM}
        const obj = comp.createObject(pagesNest, args)

        // 列表添加
        const dic = {
            obj: obj,
            info: infoList[0],
            infoIndex: 0,
            id: owner
        }
        pageList.splice(index, 0, dic) // 列表添加
    }

    // 增： 在 pageList 的 index 处，插入一个 infoList[infoIndex] 页面。
    function addPage(index, infoIndex){ // index=-1 代表尾部插入
        // 实例化页面，挂到巢下
        const comp = infoList[infoIndex].comp
        if(!comp){
            console.error("【Error】添加页面失败：下标"+index+"的组件comp不存在！")
            return
        }
        var args = {z: -1, visible: false}
        if (comp.url == "qrc:/zenographview.qml")
        {
            console.log("add nodesmodel")
            args["graphModel"] = nodesModel
            console.log(args)
        }
        const obj = comp.createObject(pagesNest, args)

        // 列表添加
        const dic = {
            obj: obj,
            info: infoList[infoIndex],
            infoIndex: infoIndex
        }
        pageList.splice(index, 0, dic) // 列表添加
    }

    // 删： 在 pageList 的 index 处，删除该页面。
    function delPage(index){
        pageList[index].obj.destroy()  // 页对象删除
        pageList.splice(index, 1)  // 列表删除
    }

    // 改： 在 pageList 的 index 处，删除该页面，改为 infoIndex 页。
    function changePage(index, infoIndex){
        // 实例化页面，挂到巢下
        const comp = infoList[infoIndex].comp
        const obj = comp.createObject(pagesNest, {z: -1, visible: false})
        // 列表替换
        const dic = {
            obj: obj,
            info: infoList[infoIndex],
            infoIndex: infoIndex
        }
        pageList[index].obj.destroy()  // 旧页对象删除
        pageList[index] = dic  // 替换新页
    }

    // 改： 展示 index 页。
    function showPage(index){
        // 遍历，将展示的页面设为可视状态，其他页面设为非可视状态
        for(let i in pageList){
            if(i==index){
                pageList[i].obj.z = 0
                pageList[i].obj.visible = true
            }else{
                pageList[i].obj.z = -1
                pageList[i].obj.visible = false
            }
        }
    }

    // 改： 将一个原本在 index 的页移到 go 处。
    function movePage(index, go){
        var x = pageList.splice(index, 1)[0] // 删除
        pageList.splice(go, 0, x) // 添加
    }

    // 查： 传入下标 index 列表 list 报错内容前缀 msg ，返回下标是否合法。
    function isIndex(index, list, msg=""){
        if(index<0 || index>=list.length){
            if(msg)
                console.error(msg+"下标"+index+"超出范围"+(pageList.length-1)+"！")
            return false
        }
        return true
    }

    // ========================= 【辅助元素】 =========================
    
    // 页巢，作为已生成的页组件对象的父级。可挂载到可视节点下来展示。
    Item {
        id: pagesNest
        anchors.fill: parent
    }
    property var pagesNest: pagesNest
}