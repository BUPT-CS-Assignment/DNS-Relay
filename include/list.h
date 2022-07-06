#include <stddef.h>

/**
 * @description: 模仿Linux中list_head实现的双向链表，用于查找
 */
typedef struct list_head {
  struct list_head *next, *prev;
} mylist_head;
#define MY_LIST_HEAD_INIT(name)                                                \
  { &(name), &name }

/**offset_of(TYPE,MEMBER) ((size_t)&((TYPE *)0)->MEMBER)*/
/**
 * @description:通过取指运算符强制转换0地址，并从0地址找到TYPE的成员MEMBER
 * @return ( (size_t) & ((entry*)p))-> node);
 * 即返回p的成员node的地址，因p为0地址，表现为从0地址开始算偏移量
 */
/**
 * @description:
 * @param ptr：所求结构体中list_head成员的地址
 * @param offsetof(member)：求list_head成员距离结构体的偏移量
 * @return
 * 以字节为单位的两者相减值，即结构体首地址，实现由结构体成员地址求结构体地址
 */
// #define container_of(ptr, type, member)                                        \
//   ({                                                                           \
//     const typeof(((type *)0)->member) *_mptr = (ptr);                          \
//     (type *)((char *)_mptr - offsetof(type, member));                          \
//   })

/**
 * @description: container_of的弱化版，失去了内核编程的严谨性以回避typeof
 * @return {*}
 */
#define mylist_entry(ptr, type, member)                                        \
  ({ (type *)((char *)ptr - offsetof(type, member)); })

/**
 * @description: 从头到尾遍历
 * @return {*}
 */
#define mylist_for_each(pos, head, type)                                       \
  for (pos = (head)->next; pos != (head); pos = pos->next)

static inline void INIT_MY_LIST_HEAD(mylist_head *list) {
  list->next = list;
  list->prev = list;
}

static inline void _mylist_add(mylist_head *_new, mylist_head *prev,
                               mylist_head *next) {
  next->prev = _new;
  _new->next = next;
  _new->prev = prev;
  prev->next = _new;
}

/**
 * @description: 链表新节点插入的头插法接口
 * @param {mylist_head} *_new 要插入的新节点
 * @param {mylist_head} *head 该链表的头节点
 * @return {*}
 */
static inline void mylist_add_head(mylist_head *_new, mylist_head *head) {
  _mylist_add(_new, head, head->next);
}

/**
 * @description: 链表新节点插入的尾插法接口
 * @param {mylist_head} *_new 要插入的新节点
 * @param {mylist_head} *head 链表的头节点
 * @return {*}
 */
static inline void mylist_add_tail(mylist_head *_new, mylist_head *head) {
  _mylist_add(_new, head->prev, head);
}

static inline void _mylist_del(mylist_head *prev, mylist_head *next) {
  next->prev = prev;
  prev->next = next;
}

/**
 * @description: 链表删除某节点的接口，被删除的节点会不可用
 * @param {mylist_head} *entry 要被删除的某节点接口
 * @return {*}
 */
static inline void mylist_del(mylist_head *entry) {
  _mylist_del(entry->prev, entry->next);

  entry->next = NULL;
  entry->prev = NULL;
}

/**
 * @description: 链表删除某节点的接口，被删除的节点会成为另一独立的头节点
 * @param {mylist_head} *entry 要被删除的某节点接口
 * @return {*}
 */
static inline void mylist_del_init(mylist_head *entry) {
  _mylist_del(entry->prev, entry->next);

  INIT_MY_LIST_HEAD(entry);
}

/**
 * @description: 链表替换某节点的接口
 * @param {mylist_head} *old 旧节点
 * @param {mylist_head} *_new 新节点
 * @return {*}
 */
static inline void mylist_replace(mylist_head *old, mylist_head *_new) {
  _new->next = old->next;
  _new->next->prev = _new;
  _new->prev = old->prev;
  _new->prev->next = _new;
}

/**
 * @description: 将某节点从自己的链表中删除出来后，插入另一新链表中（头插法）
 * @param {mylist_head} *list
 * @param {mylist_head} *head
 * @return {*}
 */
static inline void mylist_move_head(mylist_head *list, mylist_head *head) {
  _mylist_del(list->prev, list->next);
  mylist_add_head(list, head);
}

/**
 * @description: 将某节点从自己的链表中删除出来后，插入另一新链表中（尾插法）
 * @param {mylist_head} *list
 * @param {mylist_head} *head
 * @return {*}
 */
static inline void mylist_move_tail(mylist_head *list, mylist_head *head) {
  _mylist_del(list->prev, list->next);
  mylist_add_tail(list, head);
}

/**
 * @description: 链表替换某节点的接口，旧节点会成为另一独立链表的头节点
 * @param {mylist_head} *old 旧节点
 * @param {mylist_head} *_new 新节点
 * @return {*}
 */
static inline void mylist_replace_init(mylist_head *old, mylist_head *_new) {
  mylist_replace(old, _new);

  INIT_MY_LIST_HEAD(old);
}

/**
 * @description: 检验某节点entry是否为链表head的最后一个节点
 * @return {*}
 */
static inline int mylist_is_last(mylist_head *entry, const mylist_head *head) {
  return entry->next == head;
}

/**
 * @description: 检验某链表head是否为空（仅有不使用的头节点）
 * @param {mylist_head} *head
 * @return {*}
 */
static inline int mylist_empty(const mylist_head *head) {
  mylist_head *next = head->next;
  return (next == head) && (next == head->prev);
}
/**
 * @description:
 * 把某一指定节点旋转至链表的最前，使用时应注意node属于head对应的链表
 * @param {mylist_head} *node 要被至于最后的节点
 * @param {mylist_head} *head 链表头节点
 * @return {*}
 */
static inline void mylist_rotate_node_head(mylist_head *node,
                                           mylist_head *head) {
  if (!mylist_empty(head)) {
    mylist_move_head(node, head);
  }
}

/**
 * @description:
 * 把某一指定节点旋转至链表的最后，使用时应注意node属于head对应的链表
 * @param {mylist_head} *node 要被至于最后的节点
 * @param {mylist_head} *head 链表头节点
 * @return {*}
 */
static inline void mylist_rotate_node_tail(mylist_head *node,
                                           mylist_head *head) {
  if (!mylist_empty(head)) {
    mylist_move_tail(node, head);
  }
}
