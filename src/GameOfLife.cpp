
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <limits>

static const int64_t MAX_VALUE = std::numeric_limits<int64_t>::max();
static const int64_t MIN_VALUE = std::numeric_limits<int64_t>::min();
static const int64_t MAX_DRAW_VALUE = 25;
static const int64_t MIN_DRAW_VALUE = -MAX_DRAW_VALUE;

typedef std::map<int64_t, std::vector<int64_t>> LifeGrid_t;

class LifeCell
{
public:

    LifeCell(
        int64_t xValue,
        int64_t yValue
    )
        : x(xValue)
        , y(yValue)
    {}

    static void GetListOfNeighbors(
        int64_t xCoordinate,
        int64_t yCoordinate,
        std::vector<LifeCell>& neighborCells)
    {
        // When the values reach the min/max values,
        // we want them to roll over to the other side.
        int64_t previousX = xCoordinate - 1;
        if (xCoordinate == MIN_VALUE)
        {
            previousX = MAX_VALUE;
        }

        int64_t previousY = yCoordinate - 1;
        if (yCoordinate == MIN_VALUE)
        {
            previousY = MAX_VALUE;
        }

        int64_t nextX = xCoordinate + 1;
        if (xCoordinate == MAX_VALUE)
        {
            nextX = MIN_VALUE;
        }

        int64_t nextY = yCoordinate + 1;
        if (yCoordinate == MAX_VALUE)
        {
            nextY = MIN_VALUE;
        }
                
        neighborCells.push_back(LifeCell(previousX, nextY));
        neighborCells.push_back(LifeCell(previousX, yCoordinate));
        neighborCells.push_back(LifeCell(previousX, previousY));

        neighborCells.push_back(LifeCell(xCoordinate, nextY));
        neighborCells.push_back(LifeCell(xCoordinate, previousY));

        neighborCells.push_back(LifeCell(nextX, nextY));
        neighborCells.push_back(LifeCell(nextX, yCoordinate));
        neighborCells.push_back(LifeCell(nextX, previousY));
    }

    int64_t x;
    int64_t y;
};

class CellPopulation
{
public:
    CellPopulation()
    {
        m_pCurrentlyAliveCells = &m_AliveCellsA;
        m_pPreviouslyAliveCells = &m_AliveCellsB;
    };

    void AddAliveCell(
        int64_t xCoordinate,
        int64_t yCoordinate)
    {
        std::vector<int64_t>& curentlyAliveCellsAtYCoordinate = (*m_pCurrentlyAliveCells)[yCoordinate];

        // We don't want any duplicates
        if (std::find(curentlyAliveCellsAtYCoordinate.begin(), curentlyAliveCellsAtYCoordinate.end(), xCoordinate) != curentlyAliveCellsAtYCoordinate.end())
        {
            return;
        }

        curentlyAliveCellsAtYCoordinate.push_back(xCoordinate);

        AddPotentiallyChangedCell(xCoordinate, yCoordinate);

        // Now, we want to add all of the neighbors of the new alive cell to the list of cells
        // that may have potentially changed.
        std::vector<LifeCell> neighborCells;
        LifeCell::GetListOfNeighbors(xCoordinate, yCoordinate, neighborCells);

        for (auto i : neighborCells)
        {
            AddPotentiallyChangedCell(i.x, i.y);
        }
    }

    void Iterate()
    {
        SwapAliveCells();
        
        // For each iteration, we start with a cell population with no alive cells.
        m_pCurrentlyAliveCells->clear();

        std::vector<LifeCell> cellsToBeAlive;

        // Iterate through all potentially changed cells.
        for (auto nextCell : m_PotentiallyChangedCells)
        {
            // Determine if a cell should be moved to "alive" status.
            bool wasCellPreviouslyAlive = WasCellPreviouslyAlive(nextCell.x, nextCell.y);
            int aliveNeighborCount = GetPreviouslyAliveNeighborCount(nextCell.x, nextCell.y);

            if  (  (wasCellPreviouslyAlive  && aliveNeighborCount == 2)
                || (wasCellPreviouslyAlive  && aliveNeighborCount == 3)
                || (!wasCellPreviouslyAlive && aliveNeighborCount == 3))
            {
                cellsToBeAlive.push_back(nextCell);
            }
        }

        // Before we add all of the alive cells as a result of this iteration,
        // we want to clear our potentially change cells for the next iteration.
        m_PotentiallyChangedCells.clear();

        for (auto nextCellToBeAlive : cellsToBeAlive)
        {
            AddAliveCell(nextCellToBeAlive.x, nextCellToBeAlive.y);
        }
    }

    void DrawAllCurrentlyAliveCells()
    {
        DrawHeader();

        int64_t lastDrawnYValue = MAX_DRAW_VALUE + 1;

        // The alive cells are sorted from lowest to highest y value.
        // For drawing the cells (line by line), we want to work from highest to lowest y value.
        for (LifeGrid_t::reverse_iterator i = m_pCurrentlyAliveCells->rbegin(); i != m_pCurrentlyAliveCells->rend(); ++i)
        {
            int64_t yValue = i->first;

            // Skip lines that are outside our draw view.
            if (yValue > MAX_DRAW_VALUE)
            {
                continue;
            }

            if (yValue < MIN_DRAW_VALUE)
            {
                break;
            }
            
            DrawEmptyRows(lastDrawnYValue, yValue);

            DrawNonEmptyRow(yValue);

            lastDrawnYValue = yValue;
        }

        DrawEmptyRows(lastDrawnYValue, MIN_DRAW_VALUE - 1);
    }

    void PrintAllCurrentlyAliveCells(bool bLife106Format)
    {
        bool noCellsAlive = true;

        if (bLife106Format)
        {
            ClearTheScreen();
            std::cout << "#Life 1.06" << std::endl;
        }

        // Neither the spec nor format requries cells to be listed in an order.
        // So, we just list them in the order that they are stored.
        for (auto i : *m_pCurrentlyAliveCells)
        {
            int64_t yValue = i.first;
            for (auto j : m_pCurrentlyAliveCells->at(i.first))
            {
                noCellsAlive = false;

                if (!bLife106Format)
                {
                    std::cout << "(";
                }
                std::cout << j;
                if (!bLife106Format)
                {
                    std::cout << ",";
                }

                std::cout << " " << yValue;
                if (bLife106Format)
                {
                    std::cout << std::endl;
                }
                else
                {
                    std::cout << ") ";
                }
            }
        }

        std::cout << std::endl;

        if (noCellsAlive && !bLife106Format)
        {
            std::cout << "All cells are dead." << std::endl;
        }
    }

private:

    void ClearTheScreen()
    {
        // I got this from https://stackoverflow.com/questions/17335816/clear-screen-using-c
        std::cout << "\033[2J\033[1;1H";
    }

    void DrawHeader()
    {
        ClearTheScreen();

        for (int i = 0; i < -MIN_DRAW_VALUE; ++i)
        {
            std::cout << "  ";
        }
        std::cout << " " << MAX_DRAW_VALUE << std::endl;
    }

    void DrawEmptyRows(
        int64_t lastDrawnYValue, 
        int64_t nextDrawnYValue)
    {
        // Check if we cross the x-axis.
        if ((lastDrawnYValue > 0) && (nextDrawnYValue < 0))
        {
            // Crossing the X-axis requires specially handling.
            DrawEmptyRows(lastDrawnYValue, 0);
            DrawEmptyXAxis();
            DrawEmptyRows(0, nextDrawnYValue);
            return;
        }

        int64_t numberOfEmptyRows = lastDrawnYValue - nextDrawnYValue;

        while (numberOfEmptyRows > 1)
        {
            for (int i = 0; i < -MIN_DRAW_VALUE; ++i)
            {
                std::cout << "  ";
            }
            std::cout << " |" << std::endl;

            numberOfEmptyRows--;
        }
    }

    void DrawEmptyXAxis()
    {
        for (int i = MIN_DRAW_VALUE; i <= MAX_DRAW_VALUE; ++i)
        {
            std::cout << "--";
        }

        std::cout << " " << MAX_DRAW_VALUE << std::endl;
    }

    void DrawNonEmptyRow(
        int64_t yValue)
    {
        char emptyChar = ' ';
        if (yValue == 0)
        {
            // Empty cells on the X-axis will show a "-" intead of a space.
            emptyChar = '-';
        }

        // Get a sorted list of all alive cell X coordinates.
        std::vector<int64_t> aliveCellXCoordinates;

        for (auto j : m_pCurrentlyAliveCells->at(yValue))
        {
            aliveCellXCoordinates.push_back(j);
        }

        std::sort(aliveCellXCoordinates.begin(), aliveCellXCoordinates.end());

        int64_t lastDrawnX = MIN_DRAW_VALUE - 1;

        // Iterate through all alive cells along the given row.
        for (auto x : aliveCellXCoordinates)
        {
            // Check if we are strictly on one side of the Y axis.
            if ((x <= 0) || (lastDrawnX >= 0))
            {
                while (lastDrawnX < x - 1)
                {
                    std::cout << emptyChar << emptyChar;
                    lastDrawnX++;
                }

                std::cout << emptyChar << "*";
                lastDrawnX++;
            }
            else
            {
                // If we are crossing the Y-axis, 
                // we need to handle drawing a "|" at the Y-axis position of the row.
                while (lastDrawnX < -1)
                {
                    std::cout << emptyChar << emptyChar;
                    lastDrawnX++;
                }

                std::cout << " |";
                lastDrawnX++;

                while (lastDrawnX < (x - 1))
                {
                    std::cout << emptyChar << emptyChar;
                    lastDrawnX++;
                }

                std::cout << emptyChar << "*";
                lastDrawnX++;
            }
        }

        // After drawing all of the alive cells, make sure we draw the Y-axis.
        while (lastDrawnX < -1)
        {
            std::cout << emptyChar << emptyChar;
            lastDrawnX++;
        }

        if (lastDrawnX == -1)
        {
            std::cout << emptyChar << "|";
            lastDrawnX++;
        }

        // Finish drawing the X-axis
        if (yValue == 0)
        {
            int64_t deltaToLastX = MAX_DRAW_VALUE - lastDrawnX;

            while (deltaToLastX > 0)
            {
                std::cout << emptyChar << emptyChar;
                deltaToLastX--;
            }

            std::cout << " " << MAX_DRAW_VALUE;
        }

        std::cout << std::endl;
    }

    void AddPotentiallyChangedCell(
        int64_t xCoordinate,
        int64_t yCoordinate)
    {
        m_PotentiallyChangedCells.push_back(LifeCell(xCoordinate, yCoordinate));
    }

    bool WasCellPreviouslyAlive(
        int64_t xCoordinate,
        int64_t yCoordinate
    )
    {
        auto iter = m_pPreviouslyAliveCells->find(yCoordinate);

        if (iter == m_pPreviouslyAliveCells->end())
        {
            return false;
        }

        std::vector<int64_t>& vectorAtXCoordinate = iter->second;

        return (std::find(vectorAtXCoordinate.begin(), vectorAtXCoordinate.end(), xCoordinate) != vectorAtXCoordinate.end());
    }

    int GetPreviouslyAliveNeighborCount(
        int64_t xCoordinate,
        int64_t yCoordinate
    )
    {
        // Get a list of all of the given cell's neighbors so we can determine if the cell should be alive.
        std::vector<LifeCell> neighborCells;
        LifeCell::GetListOfNeighbors(xCoordinate, yCoordinate, neighborCells);

        int aliveNeighborCount = 0;

        for (auto i : neighborCells)
        {
            if (WasCellPreviouslyAlive(i.x, i.y))
            {
                aliveNeighborCount++;
            }
        }

        return aliveNeighborCount;
    }

    void SwapAliveCells()
    {
        if (m_pCurrentlyAliveCells == &m_AliveCellsA)
        {
            m_pCurrentlyAliveCells = &m_AliveCellsB;
            m_pPreviouslyAliveCells = &m_AliveCellsA;
        }
        else
        {
            m_pCurrentlyAliveCells = &m_AliveCellsA;
            m_pPreviouslyAliveCells = &m_AliveCellsB;
        }
    }

    // We need 2 sets of alive cells so that while we build the next iteration, we can remember the state of the previous iteration.
    LifeGrid_t m_AliveCellsA;
    LifeGrid_t m_AliveCellsB;
    LifeGrid_t* m_pCurrentlyAliveCells;
    LifeGrid_t* m_pPreviouslyAliveCells;
    std::vector<LifeCell> m_PotentiallyChangedCells;
};

class GameOfLife
{
public:
    GameOfLife()
    {}

    bool Start()
    {
        std::cout << "-----------------------------------------------------------------------" << std::endl;
        std::cout << "Please enter the alive coordinates." << std::endl;
        std::cout << "Enter one set of coordinates per line using the following format: <x-coordinate> <y-coordinate>" << std::endl;
        std::cout << "The max coordinate value is " << MAX_VALUE << std::endl;
        std::cout << "The min coordinate value is " << MIN_VALUE << std::endl;
        std::cout << "Enter an empty line to stop enterting coordinates." << std::endl;
        std::cout << "Cells whose coordinates are less than " << MAX_DRAW_VALUE << " units from the origin will be displayed graphically." << std::endl;
        std::cout << "Example:" << std::endl;
        std::cout << "0 1" << std::endl;
        std::cout << "1 2" << std::endl;
        std::cout << "2 0" << std::endl;
        std::cout << "2 1" << std::endl;
        std::cout << "2 2" << std::endl;
        std::cout << "-2000000000000 -2000000000000" << std::endl;
        std::cout << "-2000000000001 -2000000000001" << std::endl;
        std::cout << "<empty line>" << std::endl;
        std::cout << "-----------------------------------------------------------------------" << std::endl;
        
        if (!ProcessStartState())
        {
            return false;
        }
        
        PrintAliveCells();
        return true;
    }

    void RunOneIteration()
    {
        m_CellPopulation.Iterate();
        PrintAliveCells();
    }

    void PrintAliveCells()
    {
        m_CellPopulation.DrawAllCurrentlyAliveCells();

        std::cout << "Alive Cells:" << std::endl;
        m_CellPopulation.PrintAllCurrentlyAliveCells(false);
    }

    void PrintFinalState()
    {
        m_CellPopulation.PrintAllCurrentlyAliveCells(true);
    }

private:
    bool ProcessStartState()
    {
        std::string nextInputLine;

        while (std::getline(std::cin, nextInputLine))
        {
            // An empty string is how the user indicates they are done inputing coordinates.
            if (nextInputLine.empty())
            {
                return true;
            }

            if (!AddAliveCellFromString(nextInputLine))
            {
                std::cout << "Unable to add alive cell from string '" << nextInputLine << "'." << std::endl;
                return false;
            }
        }

        return true;
    }

    bool AddAliveCellFromString(const std::string& newString)
    {
        std::size_t separator = newString.find(" ");

        if (separator == std::string::npos)
        {
            std::cout << "Improperly formatted input '" << newString << "'. Terminating program." << std::endl;
                return false;
        }

        if (newString.find(" ", separator+1) != std::string::npos)
        {
            std::cout << "Improperly formatted input '" << newString << "'. Terminating program." << std::endl;
            return false;
        }

        std::string firstValue = newString.substr(0, separator);
        std::string secondValue = newString.substr(separator);

        int64_t newXValue;
        try
        {
            newXValue = stoll(firstValue);
        }
        catch (...)
        {
            std::cout << "Improperly formatted input '" << firstValue << "'. Terminating program." << std::endl;
            return false;
        }

        int64_t newYValue;
        try
        {
            newYValue = stoll(secondValue);
        }
        catch (...)
        {
            std::cout << "Improperly formatted input '" << secondValue << "'. Terminating program." << std::endl;
            return false;
        }

        if ((newXValue > MAX_VALUE) || (newXValue < MIN_VALUE))
        {
            std::cout << "X value '" << newXValue << "' is outside the range of acceptible values. Terminating program." << std::endl;
            return false;
        }

        if ((newYValue > MAX_VALUE) || (newYValue < MIN_VALUE))
        {
            std::cout << "Y value '" << newYValue << "' is outside the range of acceptible values. Terminating program." << std::endl;
            return false;
        }

        m_CellPopulation.AddAliveCell(newXValue, newYValue);

        return true;
    }

    CellPopulation m_CellPopulation;
};

int main()
{
    std::cout << "Game of Life" << std::endl;

    GameOfLife gameOfLife;
    if (!gameOfLife.Start())
    {
        std::cout << "Unable to start game." << std::endl;
        return -1;
    }

    const int iterationCount = 10;

    for (int i = 1; i <= iterationCount; ++i)
    {
        std::cout << std::endl << "Hit <ENTER> to run iteration " << i<< ":" << std::endl;
        getchar();
        gameOfLife.RunOneIteration();
    }

    gameOfLife.PrintFinalState();

    std::cout << std::endl;
}
