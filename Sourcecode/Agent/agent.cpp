#include "agent.h"

namespace Richard{
    // Judge whether it is a new game or not
    bool NewGame(int n, int m, int **board) {
        /* warning: board is in TA's format */ 
        int cnt = 0;
        for (int i=0; i<m; i++)
            for (int j=0; j<n; j++)
                if (board[i][j] != 0)
                    cnt++;
        return cnt < 2; // When we goes first cnt=0, Otherwise cnt=1
    }

    // Functions for Board
    Status::Status(int n_, int m_, int noX_, int noY_) :
        n(n_), m(m_), noX(noX_), noY(noY_), curState(running), cnt(0), tot(n*m-1), curPlayer(0) {
        memset(urgent, 0, sizeof(urgent));
        for (int i = 0; i < n; i++) { top[i] = 0;}
        if (noY == 0)
            top[noX]++;
        unsigned horizon_initial = (1 << ((n + 1) << 1)) | 1; // 01 (00)*n 01
        for (int i = 0; i < m; i++)
            horizon[i] = horizon_initial;
        horizon[noY] |= 1 << ((noX+1) << 1); // 加上 X的位置

        unsigned vertical_initial = (1 << ((m + 1) << 1)) | 1; // 01 (00)*m 01
        for (int i = 0; i < n; i++)
            vertical[i] = vertical_initial;
        vertical[noX] |= 1 << ((noY + 1) << 1);

        memset(diag1, 0, sizeof(diag1)); // (x, y) => (x-y+m+1, x+1)
        for (int x = -1; x <= n; x++) { // (x,-1) (x,m)
            diag1[x + 1 + m + 1] |= 1 << ((x + 1) << 1); // diag1[m+1: m+n+2]
            diag1[x - m + m + 1] |= 1 << ((x + 1) << 1); // diag1[0: n+1]
        }
        for (int y = -1; y <= m; y++) {// (-1, y) (n, y)
            diag1[-1 - y + m + 1] |= 1 << ((-1 + 1) << 1); // diag1[0: m+1]
            diag1[n - y + m + 1] |= 1 << ((n + 1) << 1); // diag1[n+1: m+n+2]
        }
        diag1[noX - noY + m + 1] |= 1 << ((noX + 1) << 1);
        
        memset(diag2, 0, sizeof(diag2)); // (x, y) => (x+y+2, x+1) 
        for (int x = -1; x <= n; x++) { // (x, -1) (x, m)
            diag2[x - 1 + 2] |= 1 << ((x + 1) << 1);
            diag2[x + m + 2] |= 1 << ((x + 1) << 1);
        }
        for (int y = -1; y <= m; y++) { // (-1, y) (n, y)
            diag2[-1 + y + 2] |= 1 << ((-1 + 1) << 1);
            diag2[n + y + 2] |= 1 << ((n + 1) << 1);
        }
        diag2[noX + noY + 2] |= 1 << ((noX + 1) << 1);
    }

    void Status::updateThreat(int x, int y) { // used by Threat_check
        if (urgent[curPlayer][x][y])
            return;
        urgent[curPlayer][x][y] = 1;
        if (urgentList[curPlayer ^ 1].size())
            return;
        if (top[x] == y - 1 && urgent[curPlayer][x][y - 1]) {
            curState = win;
        }
        if (top[x] == y) {
            urgentList[curPlayer].insert(x);
            if (urgentList[curPlayer].size() >= 2) {
                curState = win;
            }
            else if (urgent[curPlayer][x][y + 1]) {
                curState = win;
            }
        }
    }

    void Status::Threat_check(int x, int y){ // 更新格点Thread的属性
        // check horzion
        unsigned cur_horizon = horizon[y];
        for (int bias = -2; bias <= 1; bias ++) {
            unsigned char get = (cur_horizon >> ((x + bias) << 1)) & 255;
            for (int i = 0; i < 4; i++) {
                if (get == threat_buf[curPlayer][i])
                    updateThreat(x + bias - i + 2, y);
            }
        }
        // check vertical
        unsigned cur_vertical = vertical[x];
        unsigned char get = (cur_vertical >> ((y - 1) << 1)) & 255;
        if (get == threat_buf[curPlayer][0])
            updateThreat(x, y + 1);
        //  check diag1
        unsigned cur_diag1 = diag1[x - y + m + 1];
        for (int bias = -2; bias <= 1; bias++) {
            unsigned char get = (cur_diag1 >> ((x + bias) << 1)) & 255;
            for (int i = 0; i < 4; i++) {
                if (get == threat_buf[curPlayer][i]) 
                    updateThreat(x + bias - i + 2, y + bias - i + 2);
            }
        }
        // check diag2
        unsigned cur_diag2 = diag2[x + y + 2];
        for (int bias = -2; bias <= 1; bias++) {
            unsigned char get = (cur_diag2 >> ((x + bias) << 1)) & 255;
            for (int i = 0; i < 4; i++) {
                if (get == threat_buf[curPlayer][i]) 
                    updateThreat(x + bias - i + 2, y - bias + i - 2);
            }
        }
    }

    void Status::put(int x) {
        // without validity checking
        // x = -1则nothing change
        int y = top[x]; 
        horizon[y] |= (2 | curPlayer) << ((x + 1) << 1);
        vertical[x] |= (2 | curPlayer) << ((y + 1) << 1);
        diag1[x - y + m + 1] |= (2 | curPlayer) << ((x + 1) << 1);
        diag2[x+y+2] |= (2 | curPlayer) << ((x + 1) << 1);
        cnt ++;
        top[x]++;
        if (x == noX && top[x] == noY)
            top[x]++;
        if (curState == win)
            curState = lose;
        if (curState == running) {
            if (urgent[curPlayer][x][y])
                curState = win;
            else if (cnt == tot)
                curState = draw;
        }
        if (urgent[curPlayer][x][y])
            urgentList[curPlayer].remove(x);
        if (urgent[curPlayer ^ 1][x][y])
            urgentList[curPlayer ^ 1].remove(x);

        if (urgent[curPlayer][x][top[x]])
            urgentList[curPlayer].insert(x);
        if (urgent[curPlayer ^ 1][x][top[x]])
            urgentList[curPlayer ^ 1].insert(x);

        Threat_check(x, y);
        curPlayer ^= 1; // 易手
        bool canPutExist = 0;
        for (int i = 0; i < n; i++)
            if (canPut(i)) {
                canPutExist = 1;
                break;
            }
        if (!canPutExist && curState == running)
            curState = win;
    }

    int Status::evaluate() {
        switch (curState) {
        case draw:
            return 0;
            break;
        case win:
            return 1;
            break;
        case lose:
            return -1;
            break;
        default:
            assert(false);
            break;
        }
    }

    // Funcions for UCT
    Agent::Agent(int N, int M, int noX, int noY):
        nodeCnt(1), board(N, M, noX, noY), n(N), m(M), nodePool(), root(1) {
            tr[1].refresh(-1);
        }

    time_t Agent::startTime = 0;

    int Agent::newNode(int parent) {
        if (nodeCnt+1 < MAX_TREE_NODE) {
            nodeCnt++;
            tr[nodeCnt].refresh(parent);
            return nodeCnt;
        }
        if (nodePool.empty())
            return 0;
        int x = nodePool.dequeue();
        for (int i = 0; i < n; i++)
            if (tr[x].child[i])
                nodePool.enqueue(tr[x].child[i]);
        tr[x].refresh(parent);
        return x;
    }

    void Agent::delNode(int x) {
        memset(tr[x].child, 0, sizeof(tr[x].child));
        nodePool.enqueue(x);
    }

    void Agent::deleteSubTree(int x) { nodePool.enqueue(x);}

    void Agent::RootChangeTo(std::pair<int, int> decision) { // Selection
        /* decision is in TA's format*/
        int col = decision.first;
        for (int i = 0; i < n; i++)
            if (i != col && tr[root].child[i])
                deleteSubTree(tr[root].child[i]);

        if (tr[root].child[col]) {
            int preRoot = root;
            root = tr[root].child[col];
            tr[root].parent = -1;
            delNode(preRoot);
        } else {
            tr[root].refresh(-1); // use the previous root as new root
        }
        board.put(col);
    }

    void Agent::backTrack(int x, int value) { // Backpropagation
        while (x != root) { // 将叶节点的得分一路回传到根节点
            tr[x].value += value;
            tr[x].cnt++;
            x = tr[x].parent;
            value = -value;
        }
        tr[x].value += value;
        tr[x].cnt++;
    }

    int Agent::getTreeSize() {
        std::queue<int> q;
        q.push(root);
        int cnt = 0;
        while (!q.empty()) {
            int x = q.front();
            q.pop();
            cnt++;
            for (int i = 0; i < n; i++)
                if (tr[x].child[i]) 
                    q.push(tr[x].child[i]);
        }
        return cnt;
    }

    int Agent::treePolicy(int x, Status* board) { // Expansion
        while (!board->isEnd()) {
            int expandNodes[12], expandNode = 0;
            int existNodes[12], existNode = 0;
            int decision = -1;
            if (!board->haveUrgentPoint(&decision)) { //未出现Urgent Threat, 则进行扩展
                for (int i = 0; i < n; i++) { // 遍历列（竖向）-- 寻找是否还有可扩展的子节点
                    if (board->canPut(i)) { 
                        if (!tr[x].child[i]) { 
                            expandNodes[expandNode++] = i; // 尚存在未被扩展的
                        }
                        else { 
                            existNodes[existNode++] = i; // 已被扩展的
                        }
                    }
                }

                if (expandNode){ 
                    decision = expandNodes[rand() % expandNode]; // 如果有尚未扩展的随机选取一个进行扩展
                } else{ 
                    float mx = -INF;
                    for (int i = 0; i < existNode; i++) { // Best(x, c)
                        int y = tr[x].child[existNodes[i]];
                        float confident = tr[y].value * 1.0 / tr[y].cnt + Agent_C * sqrt(2 * log(tr[x].cnt * 1.0 / tr[y].cnt));
                        if (confident > mx) {
                            decision = existNodes[i];
                            mx = confident;
                        }
                    }
                    assert(decision != -1); // if (decision == -1 abort)
                }
            }
            if (!tr[x].child[decision]) { // 若当前点可以扩展新的子节点，
                int y = newNode(x); // 则扩展
                assert(y > 0);
                tr[x].child[decision] = y;
                board->put(decision);
                return y; // 并选中新扩展的点
            }
            else {
                board->put(decision);
                x = tr[x].child[decision]; // 否则 x <- BestChild 
            }
        }
        return x; // 最终可选中以当前点为根的子树中得分最高的叶节点
    }

    int Agent::defaultPolicy(int x, Status* board) { // Simulation
        int curPlayer = board->nextPlayer();
        while (!board->isEnd()) { // 随机模拟至游戏结束
            int decision;
            if (!board->haveUrgentPoint(&decision)) {  // 若出现必下位置则下，否则
                while (1) {
                    decision = rand() % n;
                    if (board->canPut(decision)) break; // 随机生成可落点位置 (valid-put)
                }
            }
            board->put(decision); // 落子，处理，易手
        }
        int delta = board->evaluate(); // 最后一层的得分
        return curPlayer == board->nextPlayer() ? delta : -delta; // 最后一层与根节点同为极大/小层则根节点得分delta, 否则得分-delta
    }

    std::pair<int, int> Agent::Search() { 
        int decision = -1;
        if (!this->board.haveUrgentPoint(&decision)) {
            int AgentStart = root; // having selected
            for (int cnt = 0;; cnt++) {
                Status board = this->board;
                int x = treePolicy(AgentStart, &board); // Expansion
                if (!x) break; // terminate as all nodes are used
                int delta = defaultPolicy(x, &board); // Simulation
                backTrack(x, delta); // Backpropagation
                if (cnt % 1000 == 0 && timeout()) break; // terminate as time out
            }
            int mx = -1;
            for (int i = 0; i < n; i++) { 
                if (tr[root].child[i] && tr[tr[root].child[i]].cnt > mx) {
                    mx = tr[tr[root].child[i]].cnt;
                    decision = i; // 选取模拟次数最大的字节点
                }
            }
            if (decision == -1) decision = board.CausalPut(); // default: 随便选择一个有效的落子点下
        }
        Point ret;
        ret.x = decision;
        ret.y = board.getTop(decision);
        last = ret;
        return std::make_pair((int)ret.x, (int)ret.y); // 返回落子位置（最终结果还需进行一次坐标变换）
    }
}