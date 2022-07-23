#include <string>
#include <iostream>
#include <sstream>
#include <array>
#include <vector>
#include <map>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

struct Jump {
    std::string from;
    std::string to;
    friend std::ostream& operator<<(std::ostream &out, const Jump& j)
    {
        out << "{from = " << std::hex << j.from << "; to = " << std::hex << j.to << '}';
        return out;
    }
};

struct Trace {
private:
    std::array<Jump, 32U> stack_;
    int size_ = 0;

public:

    int size() const {return size_; }

    bool empty() const {return size_ == 0;}

    void push_back(Jump j) {
        stack_[size_++] = j;
    }
    void pop_back() {
        size_ -= !empty();
    }

    Jump &back() {
        return stack_[size_ - 1];
    }

    const Jump &operator[](int i) const {
        return stack_[i];
    }
    Jump &operator[](int i) {
        return stack_[i];
    }

friend std::istream &operator>>(std::istream &in, Trace &trace);
friend std::ostream &operator<<(std::ostream &out, const Trace &trace);
};


using pair_string = std::pair<std::string, std::string>;
using FreqTable = std::map<pair_string, uint64_t>;


std::pair<std::string_view,std::string_view> extract_br(const std::string& str)
{
    auto pos = str.find('/');
    std::string_view fi(str.data(), pos);

    auto pos2 = str.find('/', pos + 1);
    std::string_view se(str.data() + pos + 1, pos2 - pos - 1);

    // drop +0x...

    fi = fi.substr(0, fi.find("+0x"));
    se = se.substr(0, se.find("+0x"));

    return {fi, se};
}

std::istream &operator>>(std::istream &in, Trace &trace)
{
    std::string comm, event;
    in >> comm >> event;
    std::string jump_event;
    for(;in >> jump_event;) {
        if (comm != "cc1plus") continue;

        auto [a,b] = extract_br(jump_event);

        Jump j;
        j.from = a;
        j.to = b;
        trace.push_back(j);

    }
    return in;
}

std::ostream &operator<<(std::ostream &out, const Trace &trace)
{
    for (int i = 0; i < trace.size(); i++)
    {
        out << trace[i] << " ";
    }
    return out;
}


std::vector<Trace> read_traces()
{
    std::vector<Trace> traces;
    std::string buf;
    while(std::getline(std::cin, buf)) {
        Trace t;
        std::stringstream(buf) >> t;
        traces.push_back(t);
    }
    return traces;
}

std::vector<Trace> execute()
{
    constexpr int WRITE_END = 1;
    constexpr int READ_END = 0;
    pid_t pid;
    int fd[2];
    pipe(fd);
    pid = fork();
    std::vector<Trace> traces;
    if (pid == 0) {
        dup2(fd[WRITE_END], STDOUT_FILENO);
        close(fd[READ_END]);
        close(fd[WRITE_END]);
        execlp("perf", "perf", "script", "-F", "comm,event,brstacksym", "--no-demangle", NULL);
        std::cerr << "Failed perf\n";
        exit(1);
    } else {
        dup2(fd[READ_END], STDIN_FILENO);
        close(fd[READ_END]);
        close(fd[WRITE_END]);
        traces = read_traces();
        int status;
        waitpid(pid, &status, 0);
    }
    return traces;
}

void update_table(FreqTable& table, const vector<Trace>& traces) {
    for (const auto &tr : traces) {
        for (int i = 0; i < tr.size(); i++) {
            auto &j = tr[i];
            if(j.from != "[unknown]" && j.to != "[unknown]") {
            table[{j.from, j.to}]++;
            }
        }
    }
}

int main()
{
    constexpr int N_REPEAT = 32;
    FreqTable table;
    for (int i = 0; i < N_REPEAT; i++) {
        system("perf record -e cycles:u -j call ~/gcc_patched/gcc-install-final/bin/g++ -w tramp3d-v4.cpp -o /dev/null");
        auto traces = execute();
        std::cerr << "[" << (i+1) << "/" << N_REPEAT << "] ... done\n";
        update_table(table, traces);
    }
    for (const auto &[names, freq] : table) {
        std::cout << names.first << " " << names.second << " " << freq << "\n";
    }
}

