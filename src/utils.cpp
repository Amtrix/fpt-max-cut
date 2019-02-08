#include "./utils.hpp"
#include "./colormod.hpp"
#include <sstream>
#include <string>
#include <map>
#include <experimental/filesystem>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>
#include <csignal>
using namespace std;
using namespace std::experimental;

void CustomAssertImpl_(const string file, const int line_num, bool res, const string msg) {
    if (!res) {
        cout << file << ":" << line_num << ": failed assertion." << endl;
        if (msg.size() > 0)
            cout << "Error message: " << msg << endl;
        exit(0);
    }
}

void OutputDebugLog_(const string file, const int line_num, const string str, const int flags) {
    string rfile = "";
    if (file != "") {
        for (unsigned int i = 0; i < file.size(); ++i)
            if (file[i] == '/' || file[i] == '\\') rfile = "";
            else rfile += file[i];
    }
    cout << "LOG: " << (rfile.size() > 0 ? rfile + ":" + to_string(line_num) + ":   " : "") << str << (flags & 1 ? "" : "\n");
}

void OutputDebugLogX_(const string str) {
    cout << str;
}

void OutputDebugVector_(const string file, const int line_num, const string name, const vector<int> vec) {
    OutputDebugLog_(file, line_num, name + ": ", NO_LINE_BREAK);
    for (unsigned int i = 0; i < vec.size(); ++i)
        OutputDebugLogRaw(to_string(vec[i]) + " ");
    OutputDebugLogRaw("\n");
}

void OutputDebugVector_(const string file, const int line_num, const string name, const vector<pair<int,int>> vec) {
    OutputDebugLog_(file, line_num, name + ": ", NO_LINE_BREAK);
    for (unsigned int i = 0; i < vec.size(); ++i)
        OutputDebugLogRaw("(" + to_string(vec[i].first) + "," + to_string(vec[i].second) + ") ");
    OutputDebugLogRaw("\n");
}


// source - target
// order in source preserved
vector<int> SetSubstract(const vector<int>& source, const vector<int>& target) {
    vector<int> ret;

    unordered_map<int,int> exist;
    for (unsigned int i = 0; i < target.size(); ++i)
        exist[target[i]] = true;
    
    for (unsigned int i = 0; i < source.size(); ++i)
        if (!exist[source[i]])
            ret.push_back(source[i]);

    return ret;
}

vector<int> SetUnion(const vector<int>& s1, const vector<int>& s2) {
    unordered_map<int,int> exist;
    for (auto e : s1) exist[e] = true;
    for (auto e : s2) exist[e] = true;

    vector<int> ret;
    for (auto e : s1) if (exist[e]) { ret.push_back(e); exist[e] = false; }
    for (auto e : s2) if (exist[e]) { ret.push_back(e); exist[e] = false; }
    return ret;
}

vector<int> SetIntersection(const vector<int>& s1, const vector<int>& s2) {
    unordered_map<int,int> exist;
    for (auto e : s1) exist[e]++;
    for (auto e : s2) exist[e]++;

    vector<int> ret;
    for (auto e : s1) if (exist[e] == 2) { ret.push_back(e); exist[e] = 0; }
    for (auto e : s2) if (exist[e] == 2) { ret.push_back(e); exist[e] = 0; }
    return ret;
}

bool IsASubsetOfB(const vector<int>& A, const vector<int>& B) {
    return SetSubstract(B, A).size() == (B.size() - A.size());
}

bool SameSets(const vector<int>& A, const vector<int>& B) {
    if (A.size() != B.size()) return false;

    unordered_map<int,char> cnt;
    for (auto e : A) cnt[e]++;
    for (auto e : B) {
        cnt[e]++;
        if (cnt[e] != 2) return false;
    }
    return true;
}

vector<int> VectorsAdd(const vector<int> A, const vector<int> B, bool cut_at_smaller) {
    if (!cut_at_smaller && A.size() != B.size())
        throw std::logic_error("Can't add vectors of different sizes!");
    
    int sz = min(A.size(), B.size());
    vector<int> ret;
    for (int i = 0; i < sz; ++i)
        ret.push_back(A[i] + B[i]);
    return ret;
}

vector<pair<unsigned long long,unsigned long long>> RemoveAnyMultipleEdgesAndSelfLoops(const vector<pair<unsigned long long,unsigned long long>>& elems) {
    vector<pair<unsigned long long,unsigned long long>> ret;
    map<pair<unsigned long long,unsigned long long>, bool> visi;

    for (auto e : elems) {
        if (visi[e] || e.first == e.second) continue;
        ret.push_back(e);
        visi[e] = true;
        visi[make_pair(e.second, e.first)] = true;
    }
    return ret;
}

std::function<int(void)> TakeFirstFromPairFunction(std::function<pair<int,vector<int>>(void)> func, int addval) {
    return [func,addval]() {
        return func().first  + addval;
    };
}

// Returns:
//  Average(A)
//  Average(B)
//  Average(1 - B/A)
//  Deviation of above.
tuple<double, double, double, double, int> ComputeAverageAndDeviation(vector<double> Ares, vector<double> Bres) {
   // vector<double> Ares, Bres;
    assert(Ares.size() == Bres.size());
    double mxval = 0;
    for (int i = 0; i < (int)Ares.size(); ++i) {
        mxval = max(mxval, Ares[i]);
        mxval = max(mxval, Bres[i]);
    }

    double Aavg = GetAverage(Ares);
    double Bavg = GetAverage(Bres);

    vector<double> rate;
    for (int i = 0; i < (int)Ares.size(); ++i) 
        rate.push_back((Bres[i] / Ares[i]) - 1);

    double RATEavg = GetAverage(rate);
    double SDres = GetStandardDeviation(rate);

    return make_tuple(Aavg, Bavg, RATEavg, SDres, mxval);
}

tuple<double, double, double, double, int> ComputeAverageAndDeviation(std::function<int(void)> Agen, std::function<int(void)> Bgen, int iter) {
    vector<double> A,B;
    while (iter--) {
        A.push_back(Agen());
        B.push_back(Bgen());
    }

    return ComputeAverageAndDeviation(A, B);
}

vector<string> GetAllDatasets(const string path) {
    filesystem::path cpppath = path.c_str();

    vector<string> ret;
    if (filesystem::is_directory(cpppath)) {
        for (auto & p : filesystem::directory_iterator(path))
            if (filesystem::is_regular_file(p)) {
                string fname = filesystem::path(p.path().u8string()).filename();
                if (fname == "README.md") continue;
                ret.push_back(p.path().u8string());
            }
    } else {
        ret.push_back(cpppath.u8string());
    }
    return ret;
}

const vector<string> ReadLine(std::istream& in, string *read_line) {
    vector<string> ret;

    string w;
    while (!in.eof()) {
        getline(in, w);

        if (read_line)
            *read_line = w;
    
        if (w != "") break;
    }

    stringstream line_in(w);
    while (line_in.eof() == false) {
        string val; line_in >> val;
        ret.push_back(val);
    }

    while (ret.empty() == false && ret.back() == "") ret.pop_back();
    return ret;
}

int GetANonZeroWeight(int edge_weight_lo, int edge_weight_hi) {
    int res_w = 0;
    while (res_w != 0) {
        int span = edge_weight_hi - edge_weight_lo + 1;
        int off = rand() % span;
        res_w = edge_weight_lo + off;
    }

    return res_w;
}

#define READ_POPEN2 0
#define WRITE_POPEN2 1

pid_t
popen2(const char *binfile, const char* file_read_in, int *outfp)
{
    int pipeB[2];
    pid_t pid;

    if (pipe(pipeB) != 0) {
        perror("pipe");
        return -1;
    }

    pid = fork();

    if (pid < 0) {
        cout << "Error, pid < 0" << endl;
        return pid;
    } else if (pid == 0)
    {
        close(pipeB[READ_POPEN2]);
        dup2(pipeB[WRITE_POPEN2], WRITE_POPEN2);

        if (freopen(file_read_in, "r", stdin) == NULL) {
            cout << "Error opening file." << endl;
            exit(1);
        }

        execl(binfile, NULL);
        perror("execl");
        exit(1);
    }

    if (outfp == NULL)
        close(pipeB[READ_POPEN2]);
    else
        *outfp = pipeB[READ_POPEN2];

    close(pipeB[WRITE_POPEN2]);

    return pid;
}

tuple<std::string, double>  exec_custom(const string binfile, const string rfilepath, int timelimit_seconds) {
    cout << ("Executing: " + binfile + " with runtime in seconds: " + to_string(timelimit_seconds)) << endl;
    auto t0_total = std::chrono::high_resolution_clock::now();
    double finish_time_ms;

    int pid;
    int outfp;
    if ((pid=popen2(binfile.c_str(), rfilepath.c_str(), &outfp)) <= 0)
    {
        printf("Unable to exec\n");
        exit(1);
    }

    bool killed = false;
    bool done = false;
    std::string result;

    std::thread t_read([&]{
        char buf[128];
        while (read(outfp, buf, 128) > 0) {
            result += string(buf);
        }
        done = true;
        auto t1_total = std::chrono::high_resolution_clock::now();
        finish_time_ms = std::chrono::duration_cast<std::chrono::microseconds> (t1_total - t0_total).count()/1000.;
    });

    this_thread::sleep_for(std::chrono::seconds(5));
    
    std::thread t([&]{
        while (!done) {
            auto t1_total = std::chrono::high_resolution_clock::now();
            double tott_ms = std::chrono::duration_cast<std::chrono::microseconds> (t1_total - t0_total).count()/1000.;

            if (!killed && tott_ms > timelimit_seconds * 1000) {
                //cout << "Kill " << pid << " due to timeout -- reached " << tott_ms << " milliseconds." << endl;
                kill(pid, SIGABRT);
                killed = true;
                close(outfp);
                break;
            }

            this_thread::sleep_for(std::chrono::seconds(10));
        }
    });

    t.join();
    t_read.join();
    
    if (!killed) {
        close(outfp);
    }

    cout << "RESULT FOR: " << rfilepath << " ==== " << result << " ( " << finish_time_ms << " ) " << endl;

    return make_tuple(result, killed ? -1 : finish_time_ms);
}