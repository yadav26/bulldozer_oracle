// SiteClearingSimulation.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#include <vector>

using namespace std;
struct directionblock
{
    int li;//left
    int ri;//right
    int fi;//forward
    int bi;//backward
};

enum facing {
    east,
    south,
    west,
    north,
    invalid
};

std::map<string, int> fuelUsage = { {"c",1}, {"o",1}, {"r",2}, {"t",2}, {"T",-1} };
std::map<string, int> creditCost = { {"c2b",1}, {"fuel",1}, {"unclearland", 3}, {"destProtectedTree", 10}, {"damageRepair", 2} };


struct squarblock
{
    std::map<facing, directionblock> blockdata;//e=0,s=1,w=2,n=3
};

std::map <int, squarblock> ground;
struct bulldozer {
    int gid;
    int dir;
};

directionblock findallfourE(int dir, int gidx, int maxc, int maxr)
{
    int l, r, f, b;
    //assue we alway face east
    int row = gidx / maxc;
    int col = gidx % maxc;
    if (col == 0)
    {
        b = -1;
        f = gidx + 1;
    }
    else if (col + 1 >= maxc)
    {
        //we are right boundary
        b = gidx - 1;
        f = -1;
    }
    else
    {
        b = gidx - 1;
        f = gidx + 1;
    }
    if (row == 0)
    {
        l = -1;
        r = ((row +1)* maxc) + (gidx % maxc);
    }
    else if (row + 1 >= maxr)
    {
        //we are right boundary
        l = ((row - 1) * maxc) + (gidx % maxc);
        r = -1;
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
        r = -1;
        l = gidx + 1;
    }
    else if (col + 1 >= maxc)
    {
        r = gidx -1;
        l = -1;
    }
    else
    {
        l = gidx + 1;
        r = gidx - 1;
    }

    if (row == 0)
    {
        b = -1;
        f = ((row + 1) * maxc) + (gidx % maxc);
    }
    else if (row + 1 >= maxr)
    {
        b = ((row - 1) * maxc) + (gidx % maxc);
        f = -1;
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
        f = -1;
        b = gidx + 1;
    }
    else if (col + 1 >= maxc)
    {
        //we are right boundary
        f = gidx - 1;
        b = -1;
    }
    else
    {
        f = gidx - 1;
        b = gidx + 1;
    }
    if (row == 0)
    {
        r = -1;
        l = ((row + 1) * maxc) + (gidx % maxc);
    }
    else if (row + 1 >= maxr)
    {
        r = ((row - 1) * maxc) + (gidx % maxc);
        l = -1;
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
        l = -1;
        r = gidx + 1;
    }
    else if (col + 1 >= maxc)
    {
        l = gidx - 1;
        r = -1;
    }
    else
    {
        r = gidx + 1;
        l = gidx - 1;
    }

    if (row == 0)
    {
        f = -1;
        b = ((row + 1) * maxc) + (gidx % maxc);
    }
    else if (row + 1 >= maxr)
    {
        f = ((row - 1) * maxc) + (gidx % maxc);
        b = -1;
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

void printinstructions()
{
    cout << "\n\nThe simulation has ended at your request. These are the commands  you issued : \n";
}
void reportgeneration()
{
    auto w = std::setw(29);   // for number like " 10.00" (6 chars)
    auto wb = std::setw(12);  // for the numbers with more space between them
    auto sw = std::setw(31); // for titles (math, chemistry)

    //// print the numbers above
    //cout << "         ";
    //for (int i = 1; i <= 5; ++i) std::cout << w << i;
    //std::cout << "     BEST    WORST  AVERAGE\n";

    //// print a line of *
    //std::cout << sw << "" << std::string(56, '*') << '\n';

    //cout << std::setprecision(2) << std::fixed; // precision 2, fixed

    //cout << sw << std::left << "Math" << std::right;
    //for (auto ms : array[0]) std::cout << w << ms;
    //cout << wb << math_best << wb << math_worse << wb << math_average << '\n';

    //cout << sw << std::left << "Chemistry" << std::right;
    //for (auto cs : array[1]) std::cout << w << cs;
    //cout << wb << chemistry_best << wb << chemistry_worse << wb << chemistry_average << '\n';
    cout << "\n\nThe costs for this land clearing operation were:\n";

    cout << sw << std::left << "\nItem " << std::right <<wb<<"Quantity"<< wb <<"Cost";

    cout << sw << std::left << "\ncommunication overhead" << std::right; std::cout << wb << 12345678; std::cout << wb << 44445555;

    cout << sw << std::left << "\nfuel usage" << std::right;

    cout << sw << std::left << "\nuncleared squares" << std::right;

    cout << sw << std::left << "\ndestruction of protected tree" << std::right;

    cout << sw << std::left << "\npaint damage to bulldozer" << std::right;

    cout << "\n-------";

    cout << sw << std::left << "\nTotal" << std::right << wb << "        " << wb << 12345678;

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
            for (int j = 0; j < maxw; ++j)//auto itr = vi.begin(); itr != vi.end(); ++itr)
            {
                cout << "  " << ptr[j];

                auto dbE = findallfourE(0, gid, maxw, maxd);
                auto dbS = findallfourS(1, gid, maxw, maxd);
                auto dbW = findallfourW(2, gid, maxw, maxd);
                auto dbN = findallfourN(3, gid, maxw, maxd);

                squarblock sqb;
                sqb.blockdata[facing::east] = dbE;
                sqb.blockdata[facing::south] = dbS;
                sqb.blockdata[facing::west] = dbW;
                sqb.blockdata[facing::north] = dbN;

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

}
void changebulldozerdirectiontoright()
{

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


    } while (input != "q");

    return 0;
}
