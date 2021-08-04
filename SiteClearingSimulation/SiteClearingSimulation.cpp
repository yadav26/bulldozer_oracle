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
    int li=-1;//left index
    int ri=-1;//right index
    int fi=-1;//forward index
    int bi=-1;//backward index
    char type = 'o';// land type
    int bIsvisited = false;//not cleaned
    directionblock() {}
};

//view how direction are numbered, 
//turn right will increase count
//turn left will decrease count
enum facing {
    invalid=-1,
    east=0,//0
    south=1,//1
    west=2,//2
    north=3,//3
    
};

constexpr int BULLDOZER_START_DIR_EAST = 0; //initial direction east = 0
constexpr int OUT_OF_BOUNDARY = -1; 
constexpr int TERMINATE_SIM = -2; //terminate simulation
                                 
//global dimensions of the plot once read from file input
int maxw = 0, maxd = 0;

std::map<char, int> fuelUsage = { 
                                    {'c', 1}, 
                                    {'o', 1}, 
                                    {'r', 2}, 
                                    {'t', 2}, 
                                    {'T', TERMINATE_SIM} 
                                };

std::map<string, int> creditCost = { {"c2b", 1}, {"fuel",1}, {"unclearland", 3}, {"destProtectedTree", 10}, {"damageRepair", 2} };

//individual squar unit block representation
using tDirectionsFromIndex =std::map<int, directionblock>;

//individual ground collection of all squar blocks representation 0 - 49
using tGroundLayout = std::map <int, tDirectionsFromIndex>;
//struct squarblock
//{
//    std::map<int, directionblock> blockdata;//e=0,s=1,w=2,n=3
//};

//Following is the directed graph matrix where 
//int is the index of the ground starting from 0 to 49
//each index has 4 neighbor depends on which direction Bulldozer is facing
//since bulldozer can face in 4 direction on any block, each squar has 4 different set of {left,right,front,rear}
tGroundLayout ground;

//fuel - credit and protected tree removal credit
struct tcmdCost 
{
    int damageRepair;
    int fuelCost;
    int ptree;
    tcmdCost(int x, int y, int z):damageRepair(x), fuelCost(y), ptree(z){}
};

//*************************************************************************************************
//*************************************************************************************************
//Following four functions will consturct the neighboring directed indexes - DATA PREPERATION
//
//e.g. Bulldozer is at 0 index facing EAST, then function findallfourE will   
    //   )=>>  1  2  3  4  5  6  7  8  9
    //    10 11 12 13 14 15 16 17 18 19
    //    20 21 22 23 24 25 26 27 28 29
    //    30 31 32 33 34 35 36 37 38 39
    //    40 41 42 43 44 45 46 47 48 49
// left index(li)=-1, back index(bi) = -1, Forward index(fi) = 1 and right index(ri) = 10
//
//same bulldozer if at index 0, facing South(findallfourS()), |v|  then {li= 1,ri=-1,fi=10,bi=-1}
//Same bulldozer if at index 0, facing WEST(findallfourW)   , <<=( then {li=10,ri=-1,fi=-1,bi= 1}
//Same bulldozer if at index 0, facing NORTH(findallfourN)  , |^|, then {li=-1,ri= 1,fi=-1,bi=10}
//
//All above will be repeated for 50 nodes(units) and stored in map.
//
//This will facilitate the bulldozer movement on advance command ,on based on directon and fi index
//bulldozer will move ahead.
//Also, can be helpful if we extend this to left or right shift steps
//
//*************************************************************************************************

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
        r = ((row + 1) * maxc) + (gidx % maxc);
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
        r = gidx - 1;
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

//Read input file and create the play ground to be cleaned.
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

                tDirectionsFromIndex sqb;
                sqb[0] = dbE;
                sqb[1] = dbS;
                sqb[2] = dbW;
                sqb[3] = dbN;

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
/*
Bulldozer is a squar block having postion index (gid) that can only translate to the direction (dir) its facing.
- If asked to turn left , respective dir is changed. // 0=e,1=s,2=w,3=n
- If asked to advance, gid will be updated based on forward index of that block "fi"
- saving all commands
- calculate fuelusage and credit of each command 
*/

struct bulldozer 
{
    int gid;
    int dir; // 0=e,1=s,2=w,3=n
    //initial position
    bulldozer():dir(BULLDOZER_START_DIR_EAST), gid(OUT_OF_BOUNDARY){}

    //Store all commands in Q in order they are issued.
    vector<string> cmdQ;

    //Store all commands cost respective to the cmdQ indexed.
    vector<tcmdCost> cmdcost;

    //set new direction, dir will increase on right and decrease on left turn
    void turnleft() 
    {
        if (dir > 0)
            dir = dir - 1;
        else
            dir = 3;

        cmdQ.emplace_back("turn left");
    }

    //set new direction, dir will increase on right and decrease on left turn
    void turnright() {
        if (dir < 3)
            dir = dir + 1;
        else
            dir = 0;

        cmdQ.emplace_back("turn right");
    }

    //Set squar unit is cleaned/visited, update type to o/c
    void setblockisvisited(tGroundLayout& g2c, int gid)
    {
        g2c[gid][0].bIsvisited = true;
        g2c[gid][1].bIsvisited = true;
        g2c[gid][2].bIsvisited = true;
        g2c[gid][3].bIsvisited = true;
        //update the type of the block to clear 
        g2c[gid][0].type = 'o';
        g2c[gid][1].type = 'o';
        g2c[gid][2].type = 'o';
        g2c[gid][3].type = 'o';
    }

    //Advance to the steps requested. 
    //g2c - updated ground need to clean, 
    //steps = number of units to move by Bulldozer
    void advance(tGroundLayout& g2c, string req, int steps)
    {//req "a 4" -example
        stringstream is;
        is << "advance " << steps;

        //Store command for later print
        //find all fuelusage , credits for this command
        auto cost = findcost(g2c, steps);
        cmdQ.emplace_back(is.str());

        //cout << "\ncmd = " << req << ", fuelcost : " << cost.fuelCost << ", damageRepair :" << cost.damageRepair;
        cmdcost.emplace_back(cost);
    }
    
    //Following function will move bulldozer from current position to its new position
    //New position is to find by using the map store created in data preperation - phase. 
    //New position will be the bulldozer current direction & the current units forward index in the bulldozer direction
    //If new position is out of boundary - terminate simulation
    //If new postion is a protected tree - terminate simulation
    //if new position is not a terminating position(still more steps to move) and a tree - damamge paint 
    //if new position is a rock / tree / plain fuelusage
    //find and save cost incur for this command execution 

    tcmdCost findcost(tGroundLayout& g2c, int steps)
    {
        int loss = 0;
        int fuelcost = 0;
        int ptree = 0;
        int reqSteps = steps;
        bool ifBulldozerCranked = false;
        while (steps > 0 )
        {
            //cmdQ empty - no commands issued, first move, bulldozer not cranked. 
            //still more steps but bulldozer already cranked. Next time, cmdQ will not be empty.
            gid = (cmdQ.empty() && !ifBulldozerCranked )? 0 : g2c[gid][dir].fi;
            ifBulldozerCranked = true;

            if ( g2c[gid][dir].fi == OUT_OF_BOUNDARY )
            {//this means we are going to cross the boudary - terminate simulation
                tcmdCost cc(loss, fuelcost, ptree);
                cmdcost.emplace_back(cc);
                cmdQ.emplace_back("a " + to_string(reqSteps));
                stringstream s; s << "OUT_OF_BOUNDARY persued at :" << gid;
                throw(s.str());
            }

            if (g2c[gid][dir].type == 'T')
            {
                //We hit a protected tree - terminate simulation.
                //cout << "\ngid = " << gid << ", block has protected tree\n";
                ptree = creditCost["destProtectedTree"];
                tcmdCost cc(loss, fuelcost, ptree);
                cmdcost.emplace_back(cc);
                cmdQ.emplace_back("a " + to_string(reqSteps));
                stringstream s; s<< "ProtectedTree hit at :" << gid;
                throw(s.str());
            }
            else if (g2c[gid][dir].type == 't' )
            {
                if (steps - 1 > 0) 
                {//we have more steps to move, tresspassing will incur paint damage 
                    loss += creditCost["damageRepair"];
                }
                //cleaning or passing a tree unit cost same fuelusage
                fuelcost += fuelUsage['t'];
                cout << "\ngid = " << gid << ", paint damage cost : " << creditCost["damageRepair"] << ", fuelcost : " << fuelcost;
            }
            else if (g2c[gid][dir].type == 'r')
            {
                //cleaning or passing a rocky land cost same fuelusage
                fuelcost += fuelUsage['r'];
                cout << "\ngid = " << gid << ", rock cost : " << fuelcost;
            }
            else
            {
                //plain land 'o'
                fuelcost += fuelUsage[g2c[gid][dir].type];
                cout << "\ngid = " << gid << ", plain cost : " << fuelcost;
            }

            //update steps left to move
            --steps;
            
            //move bulldozer to new squar block 
            setblockisvisited(g2c, gid);
        }
        //store all costs
        tcmdCost cc(loss, fuelcost, ptree);
        return std::move(cc);
    }

    //return communication overhead
    int getCommOverhead()
    {
        return cmdQ.size();
    }

    //summarize all fuel usage
    int getfuelUsage()
    {
        int f = 0;
        for (auto itr = cmdcost.begin(); itr != cmdcost.end(); ++itr)
        {
            f+= (*itr).fuelCost ;
        }
        return f;
    }

    //summarize all damage repair credits
    int getdamageRepair()
    {
        int f = 0;
        for (auto itr = cmdcost.begin(); itr != cmdcost.end(); ++itr)
        {
            f += (*itr).damageRepair;
        }
        return f;
    }

    //get protected trees destroyed cost, should be just one, asap we terminated sim
    int getptreecost()
    {
        int f = 0;
        for (auto itr = cmdcost.begin(); itr != cmdcost.end(); ++itr)
        {
            f += (*itr).ptree;
        }
        return f;
    }
    //print all commands in order they issued.
    void printCmdQ()
    {
        for (auto itr = cmdQ.begin(); itr != cmdQ.end(); ++itr)
        {
            cout << *itr << ", ";
        }
        cout << "quit" << endl;
    }
};

//Universal unique single bulldozer instance.
bulldozer gUniqueBulldozer;

//Wrapper
void printinstructions(string s)
{
    cout << "\n\nThe simulation has ended reason - " << s <<".\n These are the commands  you issued : \n\n";
    gUniqueBulldozer.printCmdQ();
}

int getUnclearBlocks(tGroundLayout& g2c)
{
    int uc = 0;
    for (unsigned int x = 0; x < g2c.size(); ++x)
    {
        if (g2c[x][0].type == 'T' || g2c[x][0].bIsvisited )
            continue;
        ++uc;
    }
    return uc;
}

void printFinalClearLand(tGroundLayout& g2c)
{
    cout << "\n\nFinal layout of the GROUND as below \n\n";
    auto sw = std::setw(7); // for titles 
    for (int i = 0; i < maxd; ++i)
    {
        cout << "\n\n";
        for (int j = 0; j < maxw; ++j)
        {
            int gid = i * maxw + j;
            if (g2c[gid][0].bIsvisited)
                cout << std::left << gid << "[X] " << std::right;
            else
            {
                cout << std::left << gid << "[" << g2c[gid][0].type << "] " << std::right;

            }
        }
    }
    cout << "\n";
    cout << "\n";

}

//Formated report generation for the final cleared ground
void reportgeneration(tGroundLayout&g2c)
{
    auto w = std::setw(29);   
    auto wb = std::setw(12);  // for the numbers with more space between them
    auto sw = std::setw(31); // for titles 
    int total = 0;

    cout << "\n\nThe costs for this land clearing operation were:\n";

    cout << sw << std::left << "\nItem " << std::right <<wb<<"Quantity"<< wb <<"Cost";
    std::cout << sw << "\n" << std::string(56, '=') << '\n';
    cout << sw << std::left << "\ncommunication overhead" << std::right; std::cout << wb << gUniqueBulldozer.getCommOverhead(); std::cout << wb << gUniqueBulldozer.getCommOverhead();
    total += gUniqueBulldozer.getCommOverhead();
    
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

void processCmd(tGroundLayout& g2c, string input)
{
    string cmd = input.substr(0, input.find(" "));

    if (cmd == "l" || cmd == "L")
    {
        gUniqueBulldozer.turnleft();
    }
    else if (cmd == "r"|| cmd == "R")
    {
        gUniqueBulldozer.turnright();
    }
    else if (cmd == "a"|| cmd == "A")
    {
        string steps = input.substr(input.find(" ") + 1, input.length() - 2);
        if (steps.length() > 21)
            return;
        char* p;
        long nstep = strtoll (steps.c_str(), &p, 10);
        if (*p)
        {
            cout << "\nInvalid advance command params.e.g. a 4\n";
            return;
        }

        gUniqueBulldozer.advance(g2c, input, nstep);
    }
    else if (cmd == "q"|| cmd == "Q")
    {
        printinstructions("at your request");
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
        printinstructions(s);
        reportgeneration(g2c);
    }

    printFinalClearLand(g2c);

    return 0;
}
//fixed case advance to o-o-b and hit by t
//fixed case advance to o-o-b and hit by T