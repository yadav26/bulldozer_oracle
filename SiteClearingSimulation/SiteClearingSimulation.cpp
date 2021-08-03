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
    int li=-1;//left
    int ri=-1;//right
    int fi=-1;//forward
    int bi=-1;//backward
    char type = 'o';
    int bIsvisited;
    directionblock() :bIsvisited(false) {}
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

struct tcmdCost {
    int damageRepair;
    int fuelCost;
    int ptree;
    tcmdCost(int x, int y, int z):damageRepair(x), fuelCost(y), ptree(z){}
};

struct bulldozer 
{
    int gid;
    int dir; // 0=e,1=s,2=w,3=n
    bulldozer():dir(BULLDOZER_START_DIR), gid(OUT_OF_BOUNDARY)
    {
    }
    stack<string> cmdstack;
    vector<string> cmdQ;
    vector<tcmdCost> cmdcost;
    void setdir(int d)
    {
        dir = d;
    }
    void turnleft() {
        if (dir > 0)
            dir = dir - 1;
        else
            dir = 3;

        cmdstack.push("turn left");
        cmdQ.emplace_back("turn left");
    }
    void turnright() {
        if (dir < 3)
            dir = dir + 1;
        else
            dir = 0;

        cmdstack.push("turn right");
        cmdQ.emplace_back("turn right");
    }
    bool validatedstinbounday(std::map <int, squarblock>& ground2clean, int steps)
    {
        int currpos = gid;
        int currdir = dir;
        while ( steps > 0)
        {
            if (ground2clean[gid].blockdata[dir].fi == OUT_OF_BOUNDARY)
            {
                //cout << "\nvalidatedstinbounday:OUT_OF_BOUNDARY \n";
                throw(string("OUT_OF_BOUNDARY persued."));//
                //return false;
            }
            gid = ground2clean[gid].blockdata[dir].fi;
            --steps;
        }

        //restore position
        gid = currpos;
        dir = currdir;

        return true;
    }
    void setblockisvisited(std::map <int, squarblock>& g2c, int gid)
    {
        g2c[gid].blockdata[0].bIsvisited = true;
        g2c[gid].blockdata[1].bIsvisited = true;
        g2c[gid].blockdata[2].bIsvisited = true;
        g2c[gid].blockdata[3].bIsvisited = true;
    }
    void advance(std::map <int, squarblock>& ground2clean, string req, int steps)
    {//req "a 4"
        stringstream is;
        is << "advance " << steps;

        int firstmovecost = 0;
        if (gid == -1 && cmdstack.empty())
        {//first move
            gid = 0;
            steps = steps - 1;
            if (ground2clean[0].blockdata[dir].type == 'T')
            {
                //cout << "\nFirst move is blocked. gid = " << gid << ", block has protected tree\n";
                throw(string("...oops PROTECTED_TREE on first move"));
            }
            else if (ground2clean[0].blockdata[dir].type == 't' || ground2clean[0].blockdata[dir].type == 'r')
            {
                //cout << "\nFirst move is tree gid = " << gid << ", paint damage cost : " << creditCost["damageRepair"];
                firstmovecost += creditCost["damageRepair"];
            }
            else
            {
                //visitng
                //plain land 'o'
                //top up the cost by cleaning cost of the block
                firstmovecost += fuelUsage[ground2clean[0].blockdata[dir].type];
                //cout << "\nFirst move is clear gid = " << gid << ", cost : " << fuelUsage[ground2clean[0].blockdata[dir].type];

            }
            setblockisvisited(ground2clean, 0);
        }
        cmdstack.push(is.str());
        cmdQ.emplace_back(is.str());

        auto cost = findcost(ground2clean, steps, firstmovecost);
        //cost.fuelCost += firstmovecost;
        //cout << "\ncmd = " << req << ", fuelcost : " << cost.fuelCost << ", damageRepair :" << cost.damageRepair;
        cmdcost.emplace_back(cost);
    }
    tcmdCost findcost(std::map <int, squarblock>& ground2clean, int steps, int firstmovecost)
    {
        int loss = 0;
        int fuelcost = firstmovecost;
        int ptree = 0;
        //invalidate the move if steps takes bulldozer out of range
        //validatedstinbounday(ground2clean, steps);
        int currpos = gid;
        int nextpos = ground2clean[gid].blockdata[dir].fi;
        while (steps > 0 )
        {
            if (ground2clean[gid].blockdata[dir].fi == OUT_OF_BOUNDARY)
            {
                tcmdCost cc(loss, fuelcost, ptree);
                cmdcost.emplace_back(cc);
                throw(string("OUT_OF_BOUNDARY persued."));//
            }
            gid = ground2clean[gid].blockdata[dir].fi;
            //if not a protected tree 
            if (ground2clean[gid].blockdata[dir].type == 'T')
            {
                //cout << "\ngid = " << gid << ", block has protected tree\n";
                ptree = creditCost["destProtectedTree"];
                tcmdCost cc(loss, fuelcost, ptree);
                cmdcost.emplace_back(cc);

                throw(string("...oops PROTECTED_TREE"));
            }
            else if (ground2clean[gid].blockdata[dir].type == 't' )
            {
                if (steps - 1 > 0) 
                {//only if we have more steps to move, if this is stop , we only add fuelcost to clean
                    loss += creditCost["damageRepair"];
                }
                fuelcost += fuelUsage['t'];
                //cout << "\ngid = " << gid << ", paint damage cost : " << creditCost["damageRepair"] << ", fuelcost : " << fuelcost;

            }
            else if (ground2clean[gid].blockdata[dir].type == 'r')
            {
                fuelcost += fuelUsage['r'];
                //cout << "\ngid = " << gid << ", rock cost : " << fuelcost;
            }
            else
            {
                //plain land 'o'
                //top up the cost by cleaning cost of the block
                fuelcost += fuelUsage[ground2clean[gid].blockdata[dir].type];
                //cout << "\ngid = " << gid << ", plain cost : " << fuelcost;
            }

            //we moved ahead by 1
            --steps;
            
            //move bulldozer to blockdata fi
            setblockisvisited(ground2clean, gid);
        }

        //update the type of the block to clear 
        ground2clean[gid].blockdata[dir].type = 'o';

        tcmdCost cc(loss, fuelcost, ptree);
        return std::move(cc);
    }
    int getCmdCnt()
    {
        return cmdQ.size();
    }
    int getfuelUsage()
    {
        int f = 0;
        for (auto itr = cmdcost.begin(); itr != cmdcost.end(); ++itr)
        {
            f+= (*itr).fuelCost ;
        }
        return f;
    }
    int getdamageRepair()
    {
        int f = 0;
        for (auto itr = cmdcost.begin(); itr != cmdcost.end(); ++itr)
        {
            f += (*itr).damageRepair;
        }
        return f;
    }
    int getptreecost()
    {
        int f = 0;
        for (auto itr = cmdcost.begin(); itr != cmdcost.end(); ++itr)
        {
            f += (*itr).ptree;
        }
        return f;
    }
    void printinstructions()
    {
        for (auto itr = cmdQ.begin(); itr != cmdQ.end(); ++itr)
        {
            cout << *itr << ", ";
        }
        cout << "quit" << endl;
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
    cout << "\n\nThe simulation has ended at your request. These are the commands  you issued : \n\n";
    gUniqueBulldozer.printinstructions();

}
int getUnclearBlocks(std::map <int, squarblock>& g2c)
{
    int uc = 0;
    for (unsigned int x = 0; x < g2c.size(); ++x)
    {
        if (g2c[x].blockdata[0].type == 'T' || g2c[x].blockdata[0].bIsvisited )
            continue;
        ++uc;
    }
    return uc;
}
int maxw = 0, maxd = 0;
void printFinalClearLand(std::map <int, squarblock>& g2c)
{
    for (int i = 0; i < maxd; ++i)
    {
        cout << "\n";
        for (int j = 0; j < maxw; ++j)
        {
            int gid = i * maxw + j;
            if (g2c[gid].blockdata[0].bIsvisited)
                cout << " X ";
            else
                cout <<" " << g2c[gid].blockdata[0].type << " ";
        }
    }
}
void reportgeneration(std::map <int, squarblock>&g2c)
{
    auto w = std::setw(29);   
    auto wb = std::setw(12);  // for the numbers with more space between them
    auto sw = std::setw(31); // for titles 
    int total = 0;

    cout << "\n\nThe costs for this land clearing operation were:\n";

    cout << sw << std::left << "\nItem " << std::right <<wb<<"Quantity"<< wb <<"Cost";
    std::cout << sw << "\n" << std::string(56, '=') << '\n';
    cout << sw << std::left << "\ncommunication overhead" << std::right; std::cout << wb << gUniqueBulldozer.getCmdCnt(); std::cout << wb << gUniqueBulldozer.getCmdCnt();
    total += gUniqueBulldozer.getCmdCnt();
    
    cout << sw << std::left << "\nfuel usage" << std::right; std::cout << wb << gUniqueBulldozer.getfuelUsage(); std::cout << wb << gUniqueBulldozer.getfuelUsage();
    total += gUniqueBulldozer.getfuelUsage();
    
    cout << sw << std::left << "\nuncleared squares" << std::right; std::cout << wb << getUnclearBlocks(g2c); std::cout << wb << getUnclearBlocks(g2c) * creditCost["unclearland"];
    total += getUnclearBlocks(g2c) * creditCost["unclearland"];

    cout << sw << std::left << "\ndestruction of protected tree" << std::right; std::cout << wb << gUniqueBulldozer.getptreecost() / creditCost["destProtectedTree"]; std::cout << wb << gUniqueBulldozer.getptreecost();
    total += gUniqueBulldozer.getptreecost();

    cout << sw << std::left << "\npaint damage to bulldozer" << std::right; std::cout << wb << gUniqueBulldozer.getdamageRepair()/ creditCost["damageRepair"]; std::cout << wb << gUniqueBulldozer.getdamageRepair();
    total += gUniqueBulldozer.getdamageRepair();

    std::cout << sw << "\n" << std::string(56, '-') << '\n';

    cout << sw << std::left << "\nTotal" << std::right << wb << "        " << wb << total;

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
                auto dbE = findallfourE(0, gid, maxw, maxd); dbE.type = ptr[j];
                auto dbS = findallfourS(1, gid, maxw, maxd); dbS.type = ptr[j];
                auto dbW = findallfourW(2, gid, maxw, maxd); dbW.type = ptr[j];
                auto dbN = findallfourN(3, gid, maxw, maxd); dbN.type = ptr[j];

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
void movebulldozer(std::map <int, squarblock>& g2c, string input, int steps)
{
    gUniqueBulldozer.advance(g2c, input, steps);
}
void changebulldozerdirectiontoleft()
{
    gUniqueBulldozer.turnleft();
}
void changebulldozerdirectiontoright()
{
    gUniqueBulldozer.turnright();
}
void processCmd(std::map <int, squarblock>& g2c, string input)
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
        movebulldozer(g2c, input, nstep);
    }
    else if (cmd == "q")
    {
        printinstructions();
        reportgeneration(g2c);
    }
    else
    {
        cout << "invalid cmd";
    }
}
int main(int argc, char**argv) 
{
    createground(argv[1]);
    //create copy
    auto g2c = ground;

    std::string input;
    try 
    {
        do
        {
            std::cout << "\n(l)eft, (r)ight, (a)dvance <n>, (q)uit: ";
            std::getline(std::cin, input);

            processCmd(g2c, input);

        } while (input != "q");

    }
    catch (string& s)
    {
        printinstructions();
        reportgeneration(g2c);
    }

    printFinalClearLand(g2c);

    return 0;
}
//fixed case advance oob and hit by t
//fixed case advance oob and hit by T