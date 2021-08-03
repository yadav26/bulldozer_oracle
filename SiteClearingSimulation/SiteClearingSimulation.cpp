// SiteClearingSimulation.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <stack>

using namespace std;
struct directionblock
{
    int li;//left
    int ri;//right
    int fi;//forward
    int bi;//backward
    int cost2clean;
};

enum facing {
    invalid=-1,
    east=0,//0
    south=1,//1
    west=2,//2
    north=3,//3
    
};

constexpr int BULLDOZER_START_DIR = 0;
constexpr int OUT_OF_BOUNDARY = -1;
constexpr int PROTECTED_TREE = -2;
constexpr int PLAIN_LAND_COST = 1;

std::map<char, int> fuelUsage = { {'c',1}, {'o', PLAIN_LAND_COST}, {'r',2}, {'t',2}, {'T', PROTECTED_TREE} };
std::map<string, int> creditCost = { {"c2b",1}, {"fuel",1}, {"unclearland", 3}, {"destProtectedTree", 10}, {"damageRepair", 2} };


struct squarblock
{
    std::map<int, directionblock> blockdata;//e=0,s=1,w=2,n=3
};

std::map <int, squarblock> ground;



struct bulldozer 
{
    int gid;
    int dir; // 0=e,1=s,2=w,3=n
    bulldozer():dir(BULLDOZER_START_DIR), gid(-1){}
    stack<string> cmdstack;
    vector<int> cmdcost;
    void setdir(int d)
    {
        dir = d;
    }
    void turnleft() {
        if (dir > 0)
            dir = dir - 1;
        else
            dir = 3;
    }
    void turnright() {
        if (dir < 3)
            dir = dir + 1;
        else
            dir = 0;
    }
    void advance(std::map <int, squarblock>& ground2clean, string req, int steps)
    {//req "a 4"
        if (gid == -1 && cmdstack.empty())
        {//first move
            gid = 0;
        }
        cmdstack.push(req);
        int cost = findcost(ground2clean, steps);
        cmdcost.emplace_back(cost);
    }
    int findcost(std::map <int, squarblock>& ground2clean, int steps)
    {
        int cost = 0;
        while (ground2clean[gid].blockdata[dir].fi != OUT_OF_BOUNDARY || steps > 0 )
        {
            //if not a protected tree 
            if (ground2clean[gid].blockdata[dir].cost2clean == PROTECTED_TREE)
            {
                cout << "\ngid = " << gid << ", block has protected tree\n";
                throw("...oops PROTECTED_TREE");
            }

            //top up the cost by cleaning cost of the block
            cost += ground2clean[gid].blockdata[dir].cost2clean;
            //reduce the cost of the block to clear 
            if (ground2clean[gid].blockdata[dir].cost2clean > PLAIN_LAND_COST)
                ground2clean[gid].blockdata[dir].cost2clean = PLAIN_LAND_COST;

            //we moved ahead by 1
            --steps;
            
            //move bulldozer to blockdata fi
            gid = ground2clean[gid].blockdata[dir].fi;
        }
        return cost;
    }

};


bulldozer gUniqueBulldozer;

directionblock findallfourE(int dir, int gidx, int maxc, int maxr)
{
    int l, r, f, b;
    //assue we alway face east
    int row = gidx / maxc;
    int col = gidx % maxc;
    if (col == 0)
    {
        b = OUT_OF_BOUNDARY;
        f = gidx + 1;
    }
    else if (col + 1 >= maxc)
    {
        //we are right boundary
        b = gidx - 1;
        f = OUT_OF_BOUNDARY;
    }
    else
    {
        b = gidx - 1;
        f = gidx + 1;
    }
    if (row == 0)
    {
        l = OUT_OF_BOUNDARY;
        r = ((row +1)* maxc) + (gidx % maxc);
    }
    else if (row + 1 >= maxr)
    {
        //we are right boundary
        l = ((row - 1) * maxc) + (gidx % maxc);
        r = OUT_OF_BOUNDARY;
    }
    else
    {
        l = ((row - 1) * maxc) + (gidx % maxc);
        r = ((row + 1) * maxc) + (gidx % maxc);
    }

    //cout << "\n idx=" << gidx << " l=" << l << " r=" << r << " f=" << f << " b=" << b;
    directionblock db;
    db.li = l;
    db.ri = r;
    db.fi = f;
    db.bi = b;

    return std::move(db);

}
directionblock findallfourS(int dir, int gidx, int maxc, int maxr)
{
    //    ||  1  2  3  4  5  6  7  8  9
    //    10 11 12 13 14 15 16 17 18 19
    //    20 21 22 23 24 25 26 27 28 29
    //    30 31 32 33 34 35 36 37 38 39
    //    40 41 42 43 44 45 46 47 48 49
    int l, r, f, b;
    //assue we alway face south
    int row = gidx / maxc;
    int col = gidx % maxc;
    if (col == 0)
    {
        r = OUT_OF_BOUNDARY;
        l = gidx + 1;
    }
    else if (col + 1 >= maxc)
    {
        r = gidx -1;
        l = OUT_OF_BOUNDARY;
    }
    else
    {
        l = gidx + 1;
        r = gidx - 1;
    }

    if (row == 0)
    {
        b = OUT_OF_BOUNDARY;
        f = ((row + 1) * maxc) + (gidx % maxc);
    }
    else if (row + 1 >= maxr)
    {
        b = ((row - 1) * maxc) + (gidx % maxc);
        f = OUT_OF_BOUNDARY;
    }
    else
    {
        b = ((row - 1) * maxc) + (gidx % maxc);
        f = ((row + 1) * maxc) + (gidx % maxc);
    }
   // cout << "\nS idx=" << gidx << " l=" << l << " r=" << r << " f=" << f << " b=" << b;
    directionblock db;
    db.li = l;
    db.ri = r;
    db.fi = f;
    db.bi = b;
    return std::move(db);
}
directionblock findallfourW(int dir, int gidx, int maxc, int maxr)
{
    int l, r, f, b;
    //assue we alway face west
    int row = gidx / maxc;
    int col = gidx % maxc;
    if (col == 0)
    {
        f = OUT_OF_BOUNDARY;
        b = gidx + 1;
    }
    else if (col + 1 >= maxc)
    {
        //we are right boundary
        f = gidx - 1;
        b = OUT_OF_BOUNDARY;
    }
    else
    {
        f = gidx - 1;
        b = gidx + 1;
    }
    if (row == 0)
    {
        r = OUT_OF_BOUNDARY;
        l = ((row + 1) * maxc) + (gidx % maxc);
    }
    else if (row + 1 >= maxr)
    {
        r = ((row - 1) * maxc) + (gidx % maxc);
        l = OUT_OF_BOUNDARY;
    }
    else
    {
        r = ((row - 1) * maxc) + (gidx % maxc);
        l = ((row + 1) * maxc) + (gidx % maxc);
    }

    //cout << "\n W-idx=" << gidx << " l=" << l << " r=" << r << " f=" << f << " b=" << b;
    directionblock db;
    db.li = l;
    db.ri = r;
    db.fi = f;
    db.bi = b;
    return std::move(db);
}
directionblock findallfourN(int dir, int gidx, int maxc, int maxr)
{
    //    ||  1  2  3  4  5  6  7  8  9
    //    10 11 12 13 14 15 16 17 18 19
    //    20 21 22 23 24 25 26 27 28 29
    //    30 31 32 33 34 35 36 37 38 39
    //    40 41 42 43 44 45 46 47 48 49
    int l, r, f, b;
    //assue we alway face south
    int row = gidx / maxc;
    int col = gidx % maxc;
    if (col == 0)
    {
        l = OUT_OF_BOUNDARY;
        r = gidx + 1;
    }
    else if (col + 1 >= maxc)
    {
        l = gidx - 1;
        r = OUT_OF_BOUNDARY;
    }
    else
    {
        r = gidx + 1;
        l = gidx - 1;
    }

    if (row == 0)
    {
        f = OUT_OF_BOUNDARY;
        b = ((row + 1) * maxc) + (gidx % maxc);
    }
    else if (row + 1 >= maxr)
    {
        f = ((row - 1) * maxc) + (gidx % maxc);
        b = OUT_OF_BOUNDARY;
    }
    else
    {
        f = ((row - 1) * maxc) + (gidx % maxc);
        b = ((row + 1) * maxc) + (gidx % maxc);
    }
    //cout << "\n N: idx=" << gidx << " l=" << l << " r=" << r << " f=" << f << " b=" << b;
    directionblock db;
    db.li = l;
    db.ri = r;
    db.fi = f;
    db.bi = b;
    return std::move(db);
}

int printTotal() 
{
    return 1111111;
}
void printinstructions()
{
    cout << "\n\nThe simulation has ended at your request. These are the commands  you issued : \n";
}
void reportgeneration()
{
    auto w = std::setw(29);   
    auto wb = std::setw(12);  // for the numbers with more space between them
    auto sw = std::setw(31); // for titles 

    cout << "\n\nThe costs for this land clearing operation were:\n";

    cout << sw << std::left << "\nItem " << std::right <<wb<<"Quantity"<< wb <<"Cost";

    cout << sw << std::left << "\ncommunication overhead" << std::right; std::cout << wb << 12345678; std::cout << wb << 44445555;

    cout << sw << std::left << "\nfuel usage" << std::right;

    cout << sw << std::left << "\nuncleared squares" << std::right;

    cout << sw << std::left << "\ndestruction of protected tree" << std::right;

    cout << sw << std::left << "\npaint damage to bulldozer" << std::right;

    cout << "\n-------";

    cout << sw << std::left << "\nTotal" << std::right << wb << "        " << wb << printTotal();

    cout << "\n\n\nThankyou for using the Aconex site clearing simulator.\n";

}

void createground(string fp)
{
    std::cout << "Welcome to the Aconex site clearing simulator. \nThis is a map of the site : ";

    string line;
    ifstream myfile(fp);
    if (myfile.is_open())
    {
        int gid = 0;
        int maxD = 0;
        std::vector<string> vin;
        int maxw, maxd;
        while (getline(myfile, line))
        {
            vin.emplace_back(line);
            maxw = line.size();
        }

        myfile.close();

        maxd = vin.size();

        cout << "\n\nB=)";
        for (int i = 0; i < maxd; ++i)
        {
            auto ptr = vin[i].c_str();
            for (int j = 0; j < maxw; ++j)
            {
                cout << "  " << ptr[j];
                int cost = fuelUsage[ptr[j]];
                cout << cost;
                auto dbE = findallfourE(0, gid, maxw, maxd); dbE.cost2clean = cost;
                auto dbS = findallfourS(1, gid, maxw, maxd); dbS.cost2clean = cost;
                auto dbW = findallfourW(2, gid, maxw, maxd); dbW.cost2clean = cost;
                auto dbN = findallfourN(3, gid, maxw, maxd); dbN.cost2clean = cost;

                squarblock sqb;
                sqb.blockdata[0] = dbE;
                sqb.blockdata[1] = dbS;
                sqb.blockdata[2] = dbW;
                sqb.blockdata[3] = dbN;

                ground[gid] = std::move(sqb);
                ++gid;
            }
            cout << "\n   ";
        }

        cout << "\n\n";
        cout << "The bulldozer is currently located at the Northern edge of the site, \nimmediately to the West of the site, and facing East.\n\n";
    }
    else 
        cout << "Unable to open file : " << fp;

}

void movebulldozer(int steps) 
{

}

void changebulldozerdirectiontoleft()
{
    gUniqueBulldozer.turnleft();
}
void changebulldozerdirectiontoright()
{
    gUniqueBulldozer.turnright();
}
void processCmd(string input)
{
    string cmd = input.substr(0, input.find(" "));

    if (cmd == "l")
    {
        changebulldozerdirectiontoleft();
    }
    else if (cmd == "r")
    {
        changebulldozerdirectiontoright();
    }
    else if (cmd == "a")
    {
        string steps = input.substr(input.find(" ") + 1, input.length() - 2);
        if (steps.length() > 21)
            return;
        long long nstep = stoll(steps);
        cout << "\nadvance " << nstep << endl;
        movebulldozer(nstep);
    }
    else if (cmd == "q")
    {
        printinstructions();
        reportgeneration();
    }
    else
    {
        cout << "invalid cmd";
    }
}



int main(int argc, char**argv) 
{
    createground(argv[1]);
  
    std::string input;

    do 
    {
        std::cout << "\n(l)eft, (r)ight, (a)dvance <n>, (q)uit: ";
        string input;
        std::getline(std::cin, input);

        processCmd(input);

    } while (input.find("q") != -1 );

    return 0;
}
