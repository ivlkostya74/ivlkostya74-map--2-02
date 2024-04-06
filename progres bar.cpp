

#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <wtypes.h>
#include <Windows.h>
#include <chrono>
#include <vector>
#include <random>

using namespace std::chrono_literals;

std::mutex mut;
std::once_flag flag;

class consol_parameter
{
public:
    static void SetColor(int text, int background)
    {
        SetConsoleTextAttribute(hStdOut, (WORD)((background << 4) | text));
    }
    static void SetPosition(int x, int y)
    {
        COORD point;
        point.X = x;
        point.Y = y;
        SetConsoleCursorPosition(hStdOut, point);
    }
private:
    static HANDLE hStdOut;
};
HANDLE consol_parameter::hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
class Timer
{
private:
    using clock_t = std::chrono::high_resolution_clock;
    using second_t = std::chrono::duration<double, std::ratio<1> >;

    std::string m_name;
    std::chrono::time_point<clock_t> m_beg;
    double elapsed() const
    {
        return std::chrono::duration_cast<second_t>(clock_t::now() - m_beg).count();
    }

public:
    Timer() : m_beg(clock_t::now()) { }
    Timer(std::string name) : m_name(name), m_beg(clock_t::now()) { }

    void start(std::string name) {
        m_name = name;
        m_beg = clock_t::now();
    }
    void print() const {
        std::cout << m_name << ":\t" << elapsed() * 1000 << " ms" << '\n';
    }
};

void printHeaders()
{
	std::cout << "#\t" << "id\t" << "Progress Bar\t" << "Time" << std::endl;
} 

class ProgressBar {
public:
    ProgressBar(int numberThreads, int data) : numberThreads(numberThreads), data(data)
    {
        printNumberAndId();
    };
    void calculation();

private:
    int numberThreads;
    int x0 = 15;
    int x = x0;
    int y = numberThreads;
    int data;
    int width = 15;
    bool endProgress = false;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    void printNumberAndId();
};
std::vector<ProgressBar> progressBars;

void calculationProgressBar(const int numberThread, const int countThreads)
{
    std::call_once(flag, printHeaders);
    std::unique_lock<std::mutex> ulc(mut);
std:ProgressBar pb(numberThread, countThreads * 100);
    progressBars.push_back(pb);
    ulc.unlock();
    pb.calculation();
}

void ProgressBar::printNumberAndId() {
    consol_parameter::SetPosition(0, y);
    std::cout << y << "\t" << std::this_thread::get_id() << '\n';
}

void ProgressBar::calculation() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 100);
    double step = static_cast<double>(data) / width;
    double progress = 0;
    int text = 0;
    consol_parameter::SetColor(7, text);
    while (progress < data)
    {
        std::this_thread::sleep_for(200ms);

        std::unique_lock<std::mutex> ulc(mut);
        try {
            if (dis(gen) % 3 == 0) {
                throw std::runtime_error("Error");
            }
            text = 7;
        }
        catch (const std::exception& e) {
            text = 1;
        }
        consol_parameter::SetColor(7, text);
        consol_parameter::SetPosition(x, y);
        std::cout << " ";
        progress += step;
        x += 1;
        std::this_thread::sleep_for(200ms);
        ulc.unlock();
    }
    std::unique_lock<std::mutex> ulc(mut);
    auto end = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(50ms);
    consol_parameter::SetColor(7, 0);
    consol_parameter::SetPosition(x0 + width + 5, y);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << duration.count();
}


int main() {
    int countThreads = 5;
    std::vector<std::thread> vectorThreads;
    for (int i = 0; i < countThreads; ++i)
    {
        vectorThreads.push_back(std::thread(calculationProgressBar, i + 1, countThreads));
    }
    for (int i = 0; i < countThreads; ++i)
    {
        vectorThreads.at(i).join();
    }
    consol_parameter::SetPosition(0, countThreads + 1);
    return 0;
}



