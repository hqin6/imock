/**
 * 文件名称：EList.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.10.13
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _ELIST_H
#define _ELIST_H

#include "Alloc.h"

template <typename T>
class EList;

template <typename T>
class EListNode : public Alloc
{
    public:
        friend class EList<T>;
        EListNode()
        {
            m_t = NULL;
            m_next = NULL;
        }
        EListNode(const T& t) : m_t(t)
        {
            m_next = NULL;
        }
    private:
        EListNode* m_next;
        T m_t;
};

template <typename T>
class EList
{
    public:
        EList()
        {
            m_size = 0;
            m_head = NULL;
            m_it = NULL;
            m_tail = NULL;
        }
        ~EList()
        {
            clear();
        }
        T* begin()
        { 
            m_it = m_head;
            if (! m_head) return NULL;
            return &m_head->m_t;
        }
        bool end()
        {
            return m_it == NULL;
        }
        T* next() 
        { 
            m_it = m_it->m_next;
            return &m_it->m_t;
        }
        void erase(T t)
        {
            EListNode<T>* e = m_head;
            EListNode<T>* p = NULL;
            while (e)
            {
                if (e->m_t == t)
                {
                    break;
                }
                p = e;
                e = e->m_next;
            }
            if (e)
            {// 找到了
                if (e == m_head)
                {
                    m_head = e->m_next;
                }
                if (e == m_tail)
                {
                    m_tail = p;
                }
                if (p)
                {
                    p->m_next = e->m_next;
                }
                delete e;
                m_size--;
            }
        }
        void clear()
        {
           EListNode<T>* t = m_head;
           EListNode<T>* p = t;
           while (t)
           {
               p = t->m_next;
               delete t;
               t = p;
           }
           m_head = m_tail = m_it = NULL;
           m_size = 0;
        }
        void push_back(const T& t)
        {
            EListNode<T>* n = new EListNode<T>(t);
            if (! m_tail)
            {
                m_head = m_tail = n;
            }
            else
            {
                m_tail->m_next = n;
                m_tail = n;
            }
            m_size++;
        }
        int size() {return m_size;}
    private:
        EList(const EList<T>&)
        {
        }
        EList<T>& operator = (const EList<T>&)
        { 
            return NULL;
        };
    private:
        int m_size;
        EListNode<T>* m_head;
        EListNode<T>* m_tail;
        EListNode<T>* m_it;
};

#endif
