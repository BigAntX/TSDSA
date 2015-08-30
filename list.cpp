//
//  list.cpp
//  TSDSA
//
//  Created by 楚歌 on 15/8/30.
//  Copyright © 2015年 楚歌. All rights reserved.
//

#include <iostream>

// MARK: ListNode模板类
typedef int Rank;   // 秩
#define ListNodePosi(T) ListNode<T>*    // 列表节点位置

template <typename T>   // 元素类型
class ListNode {
    
public:
    // 成员
    T data; // 数值
    ListNodePosi(T) pred;   // 前驱指针
    ListNodePosi(T) succ;   // 后继指针
    // 构造函数
    ListNode() {}   // 针对header和trailer的构造
    ListNode(T e, ListNodePosi(T) p = NULL, ListNodePosi(T) s = NULL):data(e), pred(p), succ(s) {}  // 默认构造器
    // 操作接口
    ListNodePosi(T) insertAsPred(T const& e);   // 紧靠当前节点之前插入新节点
    ListNodePosi(T) insertAsSucc(T const& e);   // 紧随当前节点之后插入新节点
    
};


// MARK: List模板类
//#include "lishNode.h"

template <typename T>   // 元素类型
class List {    // 列表模板类
    
private:
    int _size;  // 规模
    ListNodePosi(T) header; // 头哨兵
    ListNodePosi(T) trailer;    // 尾哨兵
    
protected:
    void init();    // 列表创建时的初始化
    int clear();    // 清除所有节点
    void copyNodes(ListNodePosi(T), int);   // 复制列表中自位置p起的n项
    void merge(ListNodePosi(T)&, int, List<T>&, ListNodePosi(T), int);  // 有序列表区间归并
    void mergeSort(ListNodePosi(T)&, int);  // 对从p开始连续的n个节点归并排序
    void selectionSort(ListNodePosi(T), int);   // 对从p开始的n个节点选择排序
    void insertionSort(ListNodePosi(T), int);   // 对从p开始的n个节点插入排序
    
public:
    // 构造函数
    List() { init(); } // 默认
    List(List<T> const& L); // 整体复制列表L
    List(List<T> const& L, Rank r, int n);  // 复制列表L中自第r项起的n项
    List(ListNodePosi(T) p, int n); // 复制列表中自位置p起的n项
    // 析构函数
    ~List();    // 释放（包含头、尾哨兵在内的）所有节点
    // 只读访问接口
    Rank size() const { return _size; } // 规模
    bool empty() const { return _size <= 0; }   // 判空
    ListNodePosi(T) operator[](int r) const;    // 重载，支持寻秩访问（效率低）
    ListNodePosi(T) first() const { return header->succ; }  // 首节点位置
    ListNodePosi(T) last() const { return trailer->pred; }  // 末节点位置
    bool valid(ListNodePosi(T) p)   // 判断位置p是否对外合法
    { return p && (trailer != p) && (header != p); }    // 将头尾节点等同于NULL
    int disordered() const; // 判断列表是否已排序
    ListNodePosi(T) find(T const& e) const  // 无序列表查找
    { return find(e, _size, trailer); }
    ListNodePosi(T) find(T const& e, int n, ListNodePosi(T) p) const;   // 无序区间查找
    ListNodePosi(T) search(T const& e) const    // 有序列表查找
    { return search(e, _size, trailer); }
    ListNodePosi(T) search(T const& e, int n, ListNodePosi(T) p) const; // 有序区间查找
    ListNodePosi(T) selectMax(ListNodePosi(T) p, int n);    // 在p及其前n-1个后继中选出最大者
    ListNodePosi(T) selectMax() { return selectMax(header->succ, _size); }  // 整体最大者
    // 可写访问接口
    ListNodePosi(T) insertAsFirst(T const& e);  // 将e作为首节点插入
    ListNodePosi(T) insertAsLast(T const& e);   // 将e作为末节点插入
    ListNodePosi(T) insertBefore(ListNodePosi(T) p, T const& e);    // 将e当做p的前驱插入
    ListNodePosi(T) insertAfter(ListNodePosi(T) p, T const& e);    // 将e当做p的后继插入
    T remove(ListNodePosi(T) p);    // 删除合法位置p处的节点，返回被删除节点
    void merge(List<T>& L) { merge(first(), _size, L, L.first(), L._size); } // 全列表归并
    void sort(ListNodePosi(T) p, int n);    // 列表区间排序
    void sort() { sort(first(), _size); }   // 列表整体排序
    int deduplicate();  // 无序去重
    int uniquify();     // 有序去重
    // 遍历
    void traverse(void (*)(T&));    // 遍历，依次实施visit操作（函数指针，只读或局部性修改）
    template <typename VST> // 操作器
    void traverse(VST &);   // 遍历，依次实施visit操作（函数对象，可全局性修改）
}; // List

// MARK: 列表
// MARK: 默认构造方法
template <typename T>   // 元素类型
void List<T>::init() {  // 列表初始化，在创建列表对象时统一调用
    header = new ListNode<T>;   // 创建头哨兵节点
    trailer = new ListNode<T>;  // 创建尾哨兵节点
    header->succ = trailer; header->pred = NULL;
    trailer->pred = header; trailer->succ = NULL;
    _size = 0;  // 记录规模
}

// MARK: 重载列表类的下标操作符
/***************************************************************
 * 重载下标操作符，以通过秩直接访问对应的列表节点
 * （效率低。对于列表应优先考虑通过位置访问节点）
 ***************************************************************/
template <typename T>   // 元素类型
ListNodePosi(T) List<T>::operator[](int r) const {  // assert: 0 <= r < size
    ListNodePosi(T) p = first();    // 从首节点出发
    while (0 < r--) p = p->succ;    // 顺数第r个节点即是
    return p;   // 目标节点
}

// MARK: 无序列表元素查找接口find()
/***************************************************************
 * 无序区间查找算法
 * 在p（可能是trailer）的n个（真）前驱中，找到等于e的最后者
 * 失败时，返回左边界的前驱节点（可能是header）
 * assert: 0 <= n <= rank(p) < _size
 ***************************************************************/
template <typename T>
ListNodePosi(T) List<T>::find(T const& e, int n, ListNodePosi(T) p) const {
    while (0 <= n--) {  // 对p的最近n+1个前驱（最左侧的可能是header），从右向左
        p = p->pred;    // 逐个对比，直至命中或范围越界
        if (e == p->data) {
            break;
        }
    }
    return p;
}

// MARK: 列表节点插入接口
template <typename T>
ListNodePosi(T) List<T>::insertAsFirst(T const& e) {    // e当做首节点插入
    _size++;
    return header->insertAsSucc(e);
}

template <typename T>
ListNodePosi(T) List<T>::insertAsLast(T const& e) {     // e当做末节点插入
    _size++; return trailer->insertAsPred(e);
}

template <typename T>
ListNodePosi(T) List<T>::insertBefore(ListNodePosi(T) p, T const& e) {    // e当做p的前驱插入
    _size++;
    return p->insertasPred(e);
}

template <typename T>
ListNodePosi(T) List<T>::insertAfter(ListNodePosi(T) p,  T const& e) {      // e当做p的后继插入
    _size++;
    return p->insertAsSucc(e);
}

// MARK: 前插入
/***************************************************************
 * 链表节点前驱插入算法
 * 将e紧靠当前节点之前插入于当前节点所属列表
 ***************************************************************/
template <typename T>
ListNodePosi(T) ListNode<T>::insertAsPred(T const& e) {     // 所属节点设有哨兵头节点header
    ListNodePosi(T) x = new ListNode<T>(e, pred, this); // 创建新节点
    pred->succ = x; pred = x;   // 设置正向链接
    return x;   // 返回新节点位置
}

// MARK: 后插入
/***************************************************************
 * 链表节点后继插入算法
 * 将e紧随当前节点之后插入于当前节点所属列表
 ***************************************************************/
template <typename T>
ListNodePosi(T) ListNode<T>::insertAsSucc(T const& e) { // 所属列表设有哨兵节点trailer
    ListNodePosi(T) x = new ListNode<T>(e, this, succ); // 创建新节点
    succ->pred = x; succ = x;   // 设置逆向链接
    return x;   // 返回新节点位置
}

// MARK: 基于复制的构造
/***************************************************************
 * 列表内部方法：复制列表中自位置p起的n项
 * p为待复制列表中的合法位置，且至少有n-1个真后继节点
 ***************************************************************/
template <typename T>
void List<T>::copyNodes(ListNodePosi(T) p, int n) {
    init(); // 创建头、尾哨兵节点并做初始化
    for (int i = 0; i < n; i++) {   // 将起自p得n项依次
        insertAsLast(p->data);
        p = p->succ;    // 作为末节点插入到列表中
    }
}

/***************************************************************
 * 列表构造方法：复制列表中自位置p起的n项
 ***************************************************************/
template <typename T>
List<T>::List(ListNodePosi(T) p, int n) // assert: p为合法位置，且至少有n-1个后继节点
{ copyNodes(p, n); }

/***************************************************************
 * 列表构造法：整体复制列表L
 ***************************************************************/
template <typename T>
List<T>::List(List<T> const& L) {
    copyNodes(L.first(), L._size);
}

/***************************************************************
 * 列表构造方法：复制列表L中第r项起的n项
 ***************************************************************/
template <typename T>
List<T>::List(List<T> const& L, int r, int n) { // assert: r+n <= L._size
    copyNodes(L[r], n);
}


// MARK:删除
template <typename T>
T List<T>::remove(ListNodePosi(T) p) {  // 删除合法位置p处节点返回其数值
    T e = p->data;  // 备份待删除节点的数值
    p->pred->succ = p->succ;
    p->succ->pred = p->pred;
    delete p;
    _size--;
    return e;
}

// MARK: 析构
template <typename T>
List<T>::~List() {  // 列表默认析构器
    clear();
    delete header;
    delete trailer; // 清空列表，释放头、尾哨兵节点
}

template <typename T>
int List<T>::clear() {  // 清空列表
    int oldSize = _size;
    while (0 < _size) {
        remove(header->succ);   // 反复删除首节点，直至列表变空
    }
    return oldSize;
}

// MARK: 唯一化
template <typename T>
int List<T>::deduplicate() {   // 剔除无序列表中的重复节点
    if (_size < 2)  return 0;   // 平凡列表自然无重复
    int oldSize = _size;
    ListNodePosi(T) p = first();
    Rank r = 1; // p从首节点开始
    while (trailer != (p = p->succ)) {  // 依次直到末节点
        ListNodePosi(T) q = find(p->data, r, p);    // 在p的r个真前驱中查找雷同者
        (header != q) ? remove(q) : r++;    // 若的确存在，则删除之，否则秩加一
    }   // assert: 循环过程中的任意时刻，p的所有前驱互不相同
    return oldSize - _size; // 向量规模变化量，即被删除元素总数
}

// MARK: 遍历
template <typename T>
void List<T>::traverse(void (*visit)(T&)) { // 利用函数指针机制遍历，只读或局部性修改
    ListNodePosi(T) p = header;
    while ((p = p->succ) != trailer) {
        visit(p->data);
    }
}

template <typename T>   // 元素类型
template <typename VST> // 操作器
void List<T>::traverse(VST& visit) {    // 利用函数对象机制遍历，可全局性修改
    ListNodePosi(T) p = header;
    while ((p = p->succ) != trailer) {
        visit(p->data);
    }
}
