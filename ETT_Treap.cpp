#include "bits/stdc++.h"
using namespace std;

mt19937 rng(chrono::steady_clock::now().time_since_epoch().count()); /*{{{*/
namespace Treap
{
    // subtree algregate function
    using dtype = int64_t; // type for self and agg
    dtype op(dtype x, dtype y)
    {
        return x + y;
    }

    struct treap
    {
        treap *l = 0, *r = 0, *p = 0;
        dtype self; // value of this node
        dtype agg;  // subtree aggregate of this node
        int lazy;
        int size = 0;
        int prior;
        treap(int v = 0) : self(v), prior(rand()) {};
    };

#define LAZY_UPDATE 0

    void push_down(treap *t)
    {
        if (t)
        {
            t->lazy += t->p->lazy;
        }
    }

    int get_size(treap *t)
    {
        return t ? t->size : 0;
    }

    void apply(treap *t)
    {
        if (t && LAZY_UPDATE && t->lazy)
        {
            t->self += t->lazy;
            t->agg += t->lazy;
            push_down(t->l), push_down(t->r);

            t->lazy = 0;
        }
    }

    void push_up(treap *t, treap *p)
    {
        // propagate value of t to its parent p, also set p be parent of t
        if (t)
        {
            t->p = p;
            apply(t);
            p->size += t->size;
            p->agg = op(p->agg, t->agg);
        }
    }

    void update(treap *t)
    {
        if (t)
        {
            t->size = 1;
            t->agg = t->self;
            push_up(t->l, t), push_up(t->r, t);
        }
    }

    void split(treap *t, treap *&l, treap *&r, int key, int lt = 0)
    {
        if (!t)
            return void(l = r = 0);
        apply(t);
        int pos = lt + get_size(t->l);
        if (pos <= key)
        {
            split(t->r, t->r, r, key, pos + 1), l = t;
        }
        else
        {
            split(t->l, l, t->l, key, lt), r = t;
        }
        t->p = 0;
        update(t);
    }

    void merge(treap *&t, treap *l, treap *r)
    {
        apply(l), apply(r);
        if (!l || !r)
            t = l ? l : r;
        else
        {
            if (l->prior > r->prior)
            {
                merge(l->r, l->r, r), t = l;
            }
            else
            {
                merge(r->l, l, r->l), t = r;
            }
        }
        update(t);
    }

    // find position of node p in the tree
    int locate(treap *t)
    {
        if (!t)
            return -1;
        int ans = get_size(t->l);
        while (t->p)
        {
            treap *p = t->p;
            if (t == p->r)
                ans += 1 + get_size(p->l);
            t = p;
        }
        return ans;
    }

    treap *kth(treap *t, int k, int lt = 0)
    {
        if (!t)
            return t;
        apply(t);
        int pos = lt + get_size(t->l);
        if (pos == k)
            return t;
        return pos < k ? kth(t->r, k, pos + 1) : kth(t->l, k, lt);
    }

    void cut(treap *t, treap *&t1, treap *&t2, treap *&t3, int l, int r)
    {
        split(t, t2, t3, r);
        split(t2, t1, t2, l - 1);
    }

    treap *get_root(treap *t)
    {
        while (t->p)
            t = t->p;
        return t;
    }

    ostream &operator<<(ostream &os, treap *t)
    {
        string res = "{";
        for (int i = 0; i < get_size(t); i++)
        {
            if (i)
                res += ", ";
            res += to_string(kth(t, i)->self);
        }
        return os << res + "}";
    }

    // merge mutliplate roots at once
    template <typename T, typename... C>
    T join(T &root, C &...rest)
    {
        for (auto &c : {rest...})
            merge(root, root, c);
        return root;
    }

    void insert(treap *&t, int pos, treap *z)
    {
        treap *t1, *t2;
        split(t, t1, t2, pos - 1);
        t = join(t1, z, t2);
    }

    // append new element and return the size of the array
    int append(treap *&t, treap *z)
    {
        merge(t, t, z);
        return get_size(t);
    }

    // delete an element and return the deleted element
    treap *remove(treap *&t, int pos)
    {
        if (pos >= get_size(t))
            return nullptr;
        treap *t1, *t2, *t3;
        cut(t, t1, t2, t3, pos, pos);
        t = join(t1, t3);
        return t2;
    }

    // delete a subtree of the treap
    void destroy(treap *t)
    {
        if (!t)
            return;
        destroy(t->l), destroy(t->r);
        delete t;
    }

    treap *dummy = nullptr;
}

using namespace Treap;

struct ETT
{
    int n;
    vector<treap *> loop;
    vector<unordered_map<int, treap *>> adj;
    ETT(int n) : n(n), loop(n, nullptr), adj(n)
    {
        for (int i = 0; i < n; i++)
            loop[i] = new treap();
    }

    treap *reroot(int u)
    {
        int pos = locate(loop[u]);
        treap *r = root(u);
        treap *t1, *t2;
        split(r, t1, t2, pos - 1);
        return join(t2, t1);
    }

    treap *root(int u)
    {
        return get_root(loop[u]);
    }

    bool is_connected(int u, int v)
    {
        return root(u) == root(v);
    }

    // attach v to u, return new root of u
    treap *attach(int u, int v)
    {
        assert(!is_connected(u, v));
        treap *x = reroot(u), *y = reroot(v);
        adj[u][v] = new treap(), adj[v][u] = new treap();
        return join(x, adj[u][v], y, adj[v][u]);
    }

    // dettach v from u, return new root of u
    treap *dettach(int u, int v)
    {
        assert(is_connected(u, v));
        treap *x = reroot(u);
        int l = locate(adj[u][v]), r = locate(adj[v][u]);
        treap *t1, *t2, *t3;
        cut(x, t1, t2, t3, l, r);
        cut(t2, dummy, t2, dummy, 1, r - l - 1);
        delete adj[u][v];
        delete adj[v][u];
        adj[u].erase(v), adj[v].erase(u);
        return join(t1, t3);
    }
};

void solve()
{
    int n, q;
    cin >> n >> q;
    ETT ett(n);

    while (q--)
    {
        char op;
        cin >> op;
        if (op == '-')
        {
            int u, v;
            cin >> u >> v;
            ett.dettach(u, v);
        }
        else if (op == '+')
        {
            int u, v;
            cin >> u >> v;
            ett.attach(u, v);
        }
        else if (op == '?')
        {
            int u, v;
            cin >> u >> v;
            if (ett.is_connected(u, v))
                cout << "YES!" << endl;
            else
                cout << "NO!" << endl;
        }
    }
}

int main()
{
    ios_base::sync_with_stdio(0);
    cin.tie(0);
    int t = 1;
    while (t--)
        solve();
}
