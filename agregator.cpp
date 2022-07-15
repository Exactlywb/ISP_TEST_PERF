#include <string>
#include <iostream>
#include <sstream>
#include <array>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

struct Jump {
    uint64_t from;
    uint64_t to;
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


std::istream &operator>>(std::istream &in, Trace &trace)
{
    std::string comm, event;
    in >> comm >> event;
    std::string jump_event;
    for(;in >> jump_event;) {
        Jump j; char c;
        std::stringstream(jump_event) >> std::hex >> j.from >> c >> std::hex >> j.to;
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
        execlp("perf", "perf", "script", "-F", "event,brstack,comm", NULL);
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

int main()
{
    auto traces = execute();
    for(auto tr : traces) {
        std::cout << tr << '\n';
    }
}

