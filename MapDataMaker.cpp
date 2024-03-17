#include<bits/stdc++.h>

using namespace std;

/*
    by scmmm:
    start_time=2024.3.12
    地图生成几种要素:
*/

constexpr int kBerthNum=10; //泊位数量，默认位10
constexpr int kMapSize=200; //地图大小
constexpr int kBias=20; //数组多开的空间
constexpr double kPRChangeToBarriar=0.03; //每个地图有多少概率变成障碍点，泊位的点不考虑
constexpr double kPRChangeToSea=0.1;//最开始地图中海洋区块生成的概率
// PR是概率缩写
constexpr int kBerthSize=4; //泊位大小
constexpr int kMapNum=10; //生成地图的数量
constexpr int kRobotNum=10; //机器人数量
string kMapBaseName = "map-"; //存放地图文件规则，如果第一张图就叫map-1.txt

int seed=0;
char mp[kMapSize+kBias][kMapSize+kBias];
int num_barrier=0; // 最终得到的障碍数量
int vis_map[kMapSize][kMapSize]; //每次bfs生成区块的时候记录生成的点是否被访问过
vector <pair<int,int> > berth_list;

// ‘.’ ： 空地
// ‘*’ ： 海洋
// ‘#’ ： 障碍
// ‘A’ ： 机器人起始位置，总共 10 个。
// ‘B’ ： 大小为 4*4，表示泊位的位置,泊位标号在后泊位处初始化。
#define LAND '.'
#define SEA '*'
#define BARRIER '#'
#define ROBOT_START 'A'
#define BERTH 'B'

void bfs_map(int start_x,int start_y,const int kVisitingSize,const int kTypeOfMap) // 起始位置，bfs一次生成的海洋区块个数（重复生成也计算），生成数据的类型
{
    //规范一下代码书写。
    struct Mnode //没有任何意义，但是不知道怎么起名
    {
        int x,y,val;
        bool operator<(const Mnode b)
        const {
            return val<b.val;
        }
        Mnode(int x,int y,int val)
        {
            this->x=x;
            this->y=y;
            this->val=val;
        }
    };
    //清空vis数组
    //这个bfs只有的kTypeOfMap只有两种情况，一种是SEA ，一种是Barrier
    memset(vis_map,0,sizeof(vis_map));
    vis_map[start_x][start_y]=1;
    mp[start_x][start_y]= kTypeOfMap;
    //起点必须是指定类型
    int dx[4]={1,-1,0,0};
    int dy[4]={0,0,1,-1};
    priority_queue <Mnode> q;
    const int kRandMod=100;
    q.push(Mnode(start_x,start_y,rand()%kRandMod));
    // 尽可能随机化，采用优先队列，每个点随机一个数字然后按照随机的数字大小出队
    int cnt=0; // 记录当前访问点的数量
    while(!q.empty())
    {
        //记录当前bfs点的个数(注意已经是海洋的点也要统计，也就是说这里队列可能放入了已经是海洋的点)
        cnt++;
        if(cnt>kVisitingSize) break;
        Mnode now_node = q.top();
        q.pop();
        int nowx=now_node.x;
        int nowy=now_node.y;
        for(int kmove_direction=0;kmove_direction<=3;kmove_direction++)
        {

            int newx,newy;
            newx=nowx+dx[kmove_direction];
            newy=nowy+dy[kmove_direction];
            if(newx<1||newx>kMapSize||newy<1||newy>kMapSize) continue;
            if(mp[newx][newy]==SEA && kTypeOfMap==BARRIER) continue;
            if(vis_map[newx][newy]) continue;
            vis_map[newx][newy]=1;
            q.push(Mnode(newx,newy,rand()%kRandMod));
            mp[newx][newy]=kTypeOfMap;
        }
    }
    return ;
}

void init_map() //初始化地图F
{
    /*
    生成的原理大概是这样：
    开始设置海洋
        
        1.随机kSeaBaseNum个点开始一次bfs决定海洋区块。
        2.随机kBarrierBaseNum
    */ 
    //生成海洋区块的参数  
    constexpr int kSeaBaseNum=200; //随机n个点为起点开始生成海洋。
    constexpr int kBFSSeaBlockSize=50; //在kSeaBaseNum与kBFSBlockSize较小时候，期望生成的海洋区块数是两者之积
    //生成障碍区块的参数
    constexpr int kBarrierBaseNum=150;
    constexpr int kBFSBarrierBlockSize=8;
    //初始化seed
    seed=time(NULL);
    srand(seed);
    // 每个陆地等概率随机变成海洋，先别这样写，bug有点多
    for(int ki=1;ki<=kMapSize;ki++)
    for(int kj=1;kj<=kMapSize;kj++)
    {
        double PR=abs(rand()/abs(rand()+1));
        // if(PR<=kPRChangeToSea) mp[ki][kj]=SEA;
        // else mp[ki][kj]=LAND;
        mp[ki][kj]=LAND;
    }
    for(int ki=1;ki<=kSeaBaseNum;ki++)
    {
        int x=rand()%(kMapSize+1)+1;
        int y=rand()%(kMapSize+1)+1;
        //以(x,y) 为起点bfs
        bfs_map(x,y,kBFSSeaBlockSize,SEA);
    }
    //接下来生成障碍，障碍只能在陆地上生成
    for(int ki=1;ki<=kBarrierBaseNum;ki++)
    {
        int x=rand()%(kMapSize+1)+1;
        int y=rand()%(kMapSize+1)+1;
        //以(x,y) 为起点bfs
        //起点必须是陆地
        if(mp[x][y]!=LAND) 
        {
            ki--;
            continue;
        }
        bfs_map(x,y,kBFSBarrierBlockSize,BARRIER);
    }
    //随机生成十个点，表示港口左上角坐标
    //范围 [1,197]
    for(int ki=1;ki<=kBerthNum;ki++)
    {
        int x=rand()%(kMapSize-kBerthSize+1)+1;
        int y=rand()%(kMapSize-kBerthSize+1)+1;
        // 查看此时是否冲突
        int flag_have_barth=0;
        for(int kxi=0;kxi<=3;kxi++)
        for(int kyi=0;kyi<=3;kyi++)
        {
            if(mp[x+kxi][y+kyi]==BARRIER) flag_have_barth=1;
        }
        if(flag_have_barth)
        {
            ki--;
            continue;
        }
        // 港口必须是沿海靠陆地
        // ver1.0版本简单写了，就写4*4的区块内既有陆地也有海洋就行了
        int have_land=0,have_sea=0;
        for(int kxi=0;kxi<=3;kxi++)
        for(int kyi=0;kyi<=3;kyi++)
        {
            if(mp[x+kxi][y+kyi]==LAND) have_land=1;
            if(mp[x+kxi][y+kyi]==SEA) have_sea=1; 
        }
        if(have_land&&have_sea) ; 
        else
        {
            ki--;
            continue;
        }
        for(int kxi=0;kxi<=3;kxi++)
        for(int kyi=0;kyi<=3;kyi++)
        {
            mp[x+kxi][y+kyi]=BERTH;
        }
        
    }
    //最后生成机器人。
    for(int ki=1;ki<=kRobotNum;ki++)
    {
        int x=rand()%(kMapSize+1)+1;
        int y=rand()%(kMapSize+1)+1;
        if(mp[x][y]==LAND) mp[x][y]=ROBOT_START;
        else 
        {
            ki--;
            continue;
        }
    }
    // 废案
    //  此时港口生成完毕，可以以港口为起点进行bfs使得图联通
    // int dx[4]={1,-1,0,0};
    // int dy[4]={0,0,1,-1};

}

void convert_to_map_name(int num,int startplace,char *mapname)
{
    //先转化为正序string
    constexpr int tmpsize=25;
    char tmps[tmpsize];
    int lengthnum=0;
    while(num)
    {
        tmps[++lengthnum]=num%10+'0';
        num/=10;
    }
    reverse(tmps+1,tmps+lengthnum+1);
    for(int ki=1;ki<=lengthnum;ki++)
    {
        mapname[startplace+ki-1]=tmps[ki];
    }
    mapname[startplace+lengthnum]='.';
    mapname[startplace+lengthnum+1]='t';
    mapname[startplace+lengthnum+2]='x';
    mapname[startplace+lengthnum+3]='t';
}

int main()
{
    constexpr int tmpsize=25;
    char mapname[tmpsize];
    for(int ki=0;ki<kMapBaseName.size();ki++) mapname[ki]=kMapBaseName[ki];
    int namestart=kMapBaseName.size();
    for(int ki=1;ki<=kMapNum;ki++)
    {
        convert_to_map_name(ki,namestart,mapname);
        init_map();
        freopen(mapname,"w",stdout);
        for(int ki=1;ki<=kMapSize;ki++,cout<<'\n')
        for(int kj=1;kj<=kMapSize;kj++)
        {
            cout<<mp[ki][kj];
        }
        //生成每一张地图
        fclose(stdout);
    }
}