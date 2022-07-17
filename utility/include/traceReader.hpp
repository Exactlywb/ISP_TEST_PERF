#ifndef _TRACE_READER_HPP__
#define _TRACE_READER_HPP__

#include <fstream>
#include <iostream>

class TraceStream final {
    std::string currentLine_;
    std::ifstream perfFile_;

    bool isAtEOF_ = false;
    uint64_t lineNumber_ = 0;

public:
    TraceStream (const std::string &fileName) : perfFile_ (fileName)
    {
        if (!perfFile_.is_open ())
            throw std::runtime_error ("Can't open perf script file");

        advance ();
    }

    bool isAtEOF () const { return isAtEOF_; }
    std::string getCurrentLine () const
    {
        if (isAtEOF_)
            throw std::runtime_error ("Line iterator reaches the End-of-File!");
        return currentLine_;
    }

    uint64_t getLineNumber () const { return lineNumber_; }

    void advance ()
    {
        std::string curLine;
        if (!std::getline (perfFile_, curLine)) {
            isAtEOF_ = true;
            return;
        }

        currentLine_ = curLine;
        lineNumber_++;
    }

    ~TraceStream () { perfFile_.close (); }
};

#endif
