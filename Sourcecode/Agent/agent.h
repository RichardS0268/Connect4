# ifndef __AGENT_H__
# define __AGENT_H__
#include <assert.h>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <queue>
#include <utility>

using std::min;
using std::max;

namespace Richard{
    // === Global constants ===
    const int MAX_TREE_NODE = 10000000;
    const int MAX_WIDTH = 12;
    const int MC_TIME_LIMIT = 2700; // TODO should < 3000
    const int INF = 1000000000;
    const float Agent_C = float(0.8);
    const float Agent_UNREACHED_CONFIDENT = 100; // TODO

    const unsigned char win_buf[2] = {170, 255}; // 10101010 11111111
    // === compress the state ===
    const unsigned char threat_buf[2][4] = {
        {42, 138, 162, 168}, // 00101010 10001010 10100010 10101000 => bits to int
        {63, 207, 243, 252}
    };

    struct Point {char x,y;};

    bool NewGame(int n, int m, int **Status);

    // === Struct & Class Definition ====
    struct UrgentList {
        int q[20];
        int w; // length
        UrgentList() : w(0) {}
        inline void insert(int x) { q[++w] = x;}
        void remove(int x) {
            for (int i=1; i<=w; i++)
                if (q[i] == x) {
                    std::swap(q[w], q[i]);
                    w--;
                    return;
                }
            assert(false);
        }
        inline bool empty() { return w == 0;}
        inline int item() { return q[w];}
        inline int size() { return w; }
    };

    class Status {
    private:
        enum State {win, draw, running, lose};
        void Threat_check(int x, int y);
        void updateThreat(int x, int y);
        unsigned horizon[12], vertical[12], diag1[30], diag2[30];
        int n, m, noX, noY;
        int top[12]; // 12列（竖向）的顶端
        State curState;
        int cnt, tot;
        int curPlayer;
        UrgentList urgentList[2];
        bool urgent[2][13][13];
    public:
        Status(int n, int m, int noX, int noY);
        inline bool canPut(int x) const {
            // 不越界 && (落子点是自己的urgent || 落子点不是对手urgent的下方格点)
            return (top[x] < m && (urgent[curPlayer][x][top[x]] || !urgent[curPlayer ^ 1][x][top[x] + 1]));
        }
        inline int CausalPut() const {
            for (int i = 0; i < n; i++) 
                if (top[i] < m)
                    return i;
            assert(false);
        }
        inline State getState() const { return curState;}
        inline bool isEnd() const { return getState() != running;}
        inline int getTop(int x) const { return top[x];}
        inline bool nextPlayer() const { return curPlayer;}

        int evaluate();
        void put(int x);
        bool haveUrgentPoint(int* col) {
            if (!urgentList[curPlayer].empty()) { // 走棋方有Urgent threat
                *col = urgentList[curPlayer].item();
                return 1;
            }
            if (!urgentList[curPlayer ^ 1].empty()) { // 对方有Urgent threat
                *col = urgentList[curPlayer ^ 1].item();
                return 1;
            }
            return 0;
        }
    };

    struct Node {
        int child[MAX_WIDTH];
        int cnt, value;
        int parent;
        void refresh(int parent_) {
            memset(child, 0, sizeof(child));
            cnt = value = 0;
            parent = parent_;
        }
    };

    struct NodePool {
        int q[MAX_TREE_NODE + 10];
        int h, w;
        NodePool() : h(0), w(0) {}
        void enqueue(int x) {
            h++;
            if (h > MAX_TREE_NODE)
                h = 1;
            q[h] = x;
        }

        int dequeue() {
            w++;
            if (w > MAX_TREE_NODE)
                w = 1;
            return q[w];
        }

        bool empty() { return h == w;}
    };

    class Agent {
    private:
        int nodeCnt;
        int n, m;
        Point last;
        int root;
        static time_t startTime;
        Status board;
        Node tr[MAX_TREE_NODE];
        NodePool nodePool;

        int newNode(int parent);
        void delNode(int x);
        void deleteSubTree(int x);
        void backTrack(int x, int value);
        int treePolicy(int x, Status* board);
        int defaultPolicy(int x, Status* board);
        int getTreeSize();
    public:
        Agent(int N, int M, int noX, int noY);
        // ~Agent();
        std::pair<int, int> Search();
        void RootChangeTo(std::pair<int, int> decision);
        std::pair<int, int> getLastDecision() const {
            return std::make_pair((int)last.x, (int)last.y);
        }
        static void timerStart() { startTime = clock();}
        static bool timeout() {
            clock_t endTime = clock();
            return endTime - startTime > MC_TIME_LIMIT;
        }
    };
}

# endif