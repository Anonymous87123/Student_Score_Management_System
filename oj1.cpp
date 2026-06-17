#include <bits/stdc++.h>
using namespace std;
const int UP = 0;
const int RIGHT = 1;
const int DOWN = 2;
const int LEFT = 3;
const int dx[4] = {-1, 0, 1, 0};
const int dy[4] = {0, 1, 0, -1};
int seed = 0;
 
int getRandom() {
    seed = (25173 * seed + 13849) % 65536;
    return seed;
}
 
vector<vector<char>> mazeGenerator(int M, int N) {
    vector<vector<char>> maze(M, vector<char>(N, '#'));
    maze[0][1] = '.';
    maze[M-1][N-2] = '.';
    for (int i = 1; i < M - 1; i++) {
        for (int j = 1; j < N - 1; j++) {
            int r = getRandom();
            if (r < 40000) {
                maze[i][j] = '.';
            }
        }
    }
    return maze;
}
void printMaze(const vector<vector<char>>& maze, int step, bool isForward) {
    cout << (isForward ? "前进-" : "回退-") << "步骤" << step << "：" << endl;
    for (int i = 0; i < maze.size(); i++) {
        for (int j = 0; j < maze[i].size(); j++) {
            cout << maze[i][j];
        }
        cout << endl;
    }
    cout << endl;
}
int turnRight(int dir) {
    return (dir + 1) % 4;
}
int turnLeft(int dir) {
    return (dir + 3) % 4;
}
int turnBack(int dir) {
    return (dir + 2) % 4;
}
bool getNextPos(int& x, int& y, int dir, int M, int N) {
    int newX = x + dx[dir];
    int newY = y + dy[dir];
    if (newX < 0 || newX >= M || newY < 0 || newY >= N) {
        return false;
    }
    x = newX;
    y = newY;
    return true;
}
bool mazeTraverse(vector<vector<char>>& maze, int x, int y, int M, int N, 
                  int dir, int& step, bool& found) {
    if (x == M-1 && y == N-2) {
        maze[x][y] = 'X';
        printMaze(maze, step++, true);
        found = true;
        return true;
    }
    if (maze[x][y] == '.') {
        maze[x][y] = 'X';
        printMaze(maze, step++, true);
    }
    int currX = x, currY = y;
     
    int directions[4];
    directions[0] = turnRight(dir);
    directions[1] = dir;
    directions[2] = turnLeft(dir);
    directions[3] = turnBack(dir);
    for (int i = 0; i < 4; i++) {
        int newX = currX, newY = currY;
        if (getNextPos(newX, newY, directions[i], M, N)) {
            if (maze[newX][newY] == '.') {
                mazeTraverse(maze, newX, newY, M, N, directions[i], step, found);
                if (found) {
                    return true;
                }
            }
        }
    }
    if (maze[currX][currY] == 'X') {
        maze[currX][currY] = '.';
        printMaze(maze, step++, false);
    }
    return false;
}
int main() {
    int M, N;
    while (cin >> M >> N) {
        seed = 0;
        vector<vector<char>> maze = mazeGenerator(M, N);
        int step = 1;
        bool found = false;
        mazeTraverse(maze, 0, 1, M, N, DOWN, step, found);
        if (found) {
            cout << "成功走出迷宫" << endl;
        } else {
            cout << "回退到入口" << endl;
        }
    }
    return 0;
}
