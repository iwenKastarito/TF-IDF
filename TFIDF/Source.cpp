/*
 * Hey there! I’m iwenKastarito (internet name, つ ◕_◕༽つ)!
 * This code is yours to do *anything* with—use, tweak, share, whatever makes you smile!
 * See the LICENSE file or https://github.com/iwenKastarito for the full MIT License. Have fun!
 */






#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
#include <cctype>



//////////////////////////////////////////// Simple list of unimportant worsds
const std::set<std::string> STOPWORDS = {
    "and","the","is","in","at","of","a","to","it","for","on","with","as","by","that","this"
};





static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
void setColor(WORD color) {
    SetConsoleTextAttribute(hConsole, color);
}



////////////////////////////////////     extract filename from full path
std::string getFileName(const std::string& path) {
    size_t pos = path.find_last_of("\\/");
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}




//.................            convert string to lowercase
std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}







void tokenize(const std::string& text, std::vector<std::string>& tokens) {
    std::string token;
    for (char c : text) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            token += std::tolower(c);
        }
        else {
            if (!token.empty()) {
                if (!STOPWORDS.count(token))
                    tokens.push_back(token);
                token.clear();
            }
        }
    }
    if (!token.empty() && !STOPWORDS.count(token))
        tokens.push_back(token);
}
 



WORD gradientColor(double norm) {
    if (norm < 0.33)      return FOREGROUND_RED | FOREGROUND_INTENSITY;              // Red
    else if (norm < 0.66) return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; // Yellow
    else                  return FOREGROUND_GREEN | FOREGROUND_INTENSITY;           // Green
}





// File selection dialog
bool open_file_dialog(std::vector<std::string>& out_paths) {
    const DWORD MAX_BUF = 8192;
    static char buffer[MAX_BUF];
    ZeroMemory(buffer, sizeof(buffer));
    OPENFILENAMEA ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "Text Files\0*.txt;*.md\0All Files\0*.*\0";
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = MAX_BUF;
    ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_FILEMUSTEXIST;
    ofn.lpstrTitle = "Select text documents";
    if (!GetOpenFileNameA(&ofn)) return false;

    std::string first = buffer;
    char* p = buffer + first.size() + 1;
    if (*p == '\0') {
        out_paths.push_back(first);
    }
    else {
        std::string folder = first;
        while (*p) {
            out_paths.push_back(folder + "\\" + p);
            p += std::strlen(p) + 1;
        }
    }
    return true;
}

int main() {

    SetConsoleOutputCP(CP_UTF8);

    setColor(14); // Yellow
    std::cout << "\n                                        === TF-IDF Document Analysis Tool ===\n";
    setColor(7); // Default white
    std::cout << "This tool analyzes text documents, computes TF-IDF scores, cosine similarities,\n";
    std::cout << "and evaluates Precision, Recall, and F1-Score for a query term.\n\n";

    ///////////////////////////////////////////                             Select files
    std::vector<std::string> paths;
    std::cout << "Please select one or more text files (.txt or .md) via the file dialog.\n";
    if (!open_file_dialog(paths) || paths.empty()) {
        setColor(12); // Red for error
        std::cout << "Error: No files selected. Exiting.\n";
        setColor(7);
        return 0;
    }
    int N = static_cast<int>(paths.size());

    ///////////////////////////////////////////////                    Display selected files
    setColor(11); // ;ight blue
    std::cout << "\nSelected Files:\n";
    setColor(7);
    std::vector<std::string> names(N);
    for (int i = 0; i < N; ++i) {
        names[i] = getFileName(paths[i]);
        std::cout << "  " << (i + 1) << ": " << names[i] << "\n";
    }
    std::cout << "\n";

    ////////////////////////////////////////////////               Read, preprocess, compute
    std::vector<std::vector<std::string>> docs(N);
    std::vector<std::map<std::string, double>> tf(N);
    std::map<std::string, int> df;
    for (int i = 0; i < N; ++i) {
        std::ifstream in(paths[i]);
        if (!in.is_open()) {
            setColor(12);
            std::cout << "Error: Could not open file " << names[i] << ". Skipping.\n";
            setColor(7);
            continue;
        }
        std::stringstream buf;
        buf << in.rdbuf();
        tokenize(buf.str(), docs[i]);
        for (auto& t : docs[i]) tf[i][t] += 1.0;
        double maxf = 0;
        for (auto& p : tf[i]) maxf = std::max(maxf, p.second);
        for (auto& p : tf[i]) p.second /= (maxf < 1e-9 ? 1.0 : maxf);
        std::set<std::string> seen;
        for (auto& t : docs[i]) if (!seen.count(t)) {
            df[t]++; seen.insert(t);
        }
    }

    ///////////////////////////////////////////// Compute IDF and TF-IDF, collect all terms
    std::map<std::string, double> idf;
    for (auto& p : df) idf[p.first] = std::log(static_cast<double>(N) / p.second);
    std::set<std::string> terms;
    std::vector<std::map<std::string, double>> tfidf(N);
    for (int i = 0; i < N; ++i) {
        for (auto& p : tf[i]) {
            tfidf[i][p.first] = p.second * idf[p.first];
            terms.insert(p.first);
        }
    }

    /////////////////////////////////////////////   Find min/max TF-IDF for color normalization   /////////////////////////////////////////////////
    double minv = 1e9, maxv = 0;
    for (auto& t : terms) {
        for (int i = 0; i < N; ++i) {
            double v = tfidf[i].count(t) ? tfidf[i][t] : 0.0;
            minv = std::min(minv, v);
            maxv = std::max(maxv, v);
        }
    }
    double range = std::max(1e-6, maxv - minv);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



    /////////////////////////////////////////////////////        Print TF-IDF table
    setColor(14);
    std::cout << "\n                                        === TF-IDF Scores ===\n";
    setColor(7);
    std::cout << "This table shows the TF-IDF scores for each term in each document.\n";
    std::cout << "Higher scores indicate greater term importance.\n\n";

    //////////////////////////////////////////////     Compute column widths for a better looking table(as if anybody will see this)  /////////////////////   
    size_t term_col_width = 4; 
    for (const auto& term : terms) {
        term_col_width = std::max(term_col_width, term.length());
    }
    term_col_width += 2; // Add padding

    std::vector<size_t> doc_col_widths(N);
    for (int i = 0; i < N; ++i) {
        doc_col_widths[i] = names[i].length();
        for (const auto& term : terms) {
            double v = tfidf[i].count(term) ? tfidf[i][term] : 0.0;
            std::stringstream ss;
            ss << std::fixed << std::setprecision(4) << v;
            doc_col_widths[i] = std::max(doc_col_widths[i], ss.str().length());
        }
        doc_col_widths[i] += 2; // Add padding
    }

    /////////////////////////////////////////////////////////////////////     Print table    ///////////////////////////////////////////////////////
    setColor(11); // firozehee(Cyan-light blue) for TF-IDF table
    std::cout << "+";
    std::cout << std::string(term_col_width, '-') << "+";
    for (int i = 0; i < N; ++i) {
        std::cout << std::string(doc_col_widths[i], '-') << "+";
    }
    std::cout << "\n|";
    std::cout << std::left << std::setw(term_col_width) << "Term" << "|";
    for (int i = 0; i < N; ++i) {
        std::cout << std::left << std::setw(doc_col_widths[i]) << names[i] << "|";
    }
    std::cout << "\n+";
    std::cout << std::string(term_col_width, '-') << "+";
    for (int i = 0; i < N; ++i) {
        std::cout << std::string(doc_col_widths[i], '-') << "+";
    }
    std::cout << "\n";
    setColor(7);

    for (const auto& term : terms) {
        std::cout << "| " << std::left << std::setw(term_col_width - 1) << term << "|";
        for (int i = 0; i < N; ++i) {
            double v = tfidf[i].count(term) ? tfidf[i][term] : 0.0;
            double norm = (v - minv) / range;
            setColor(gradientColor(norm));
            std::cout << std::left << std::setw(doc_col_widths[i]) << std::fixed << std::setprecision(4) << v << "|";
        }
        setColor(7);
        std::cout << "\n";
    }
    setColor(11);
    std::cout << "+";
    std::cout << std::string(term_col_width, '-') << "+";
    for (int i = 0; i < N; ++i) {
        std::cout << std::string(doc_col_widths[i], '-') << "+";
    }
    std::cout << "\n\n";

    ///////////////////////////////////////////////////////   display cosine similarities
    setColor(14);
    std::cout << "                                        === Cosine Similarities ===\n";
    setColor(7);
    std::cout << "This section shows how similar each pair of documents is based on their TF-IDF vectors.\n";
    std::cout << "Values range from 0 (no similarity) to 1 (identical).\n\n";

    std::vector<WORD> docColor = {
        FOREGROUND_RED | FOREGROUND_INTENSITY,
        FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        FOREGROUND_BLUE | FOREGROUND_INTENSITY,
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
    };

    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            double dot = 0.0, ni = 0.0, nj = 0.0;
            for (auto& p : tfidf[i]) {
                auto it = tfidf[j].find(p.first);
                if (it != tfidf[j].end()) dot += p.second * it->second;
            }
            for (auto& p : tfidf[i]) ni += p.second * p.second;
            for (auto& p : tfidf[j]) nj += p.second * p.second;
            double cosim = dot / (std::sqrt(ni) * std::sqrt(nj) + 1e-12);

            setColor(docColor[i % docColor.size()]);
            std::cout << names[i];
            setColor(7);
            std::cout << " vs ";
            setColor(docColor[j % docColor.size()]);
            std::cout << names[j] << ": ";
            setColor(gradientColor(cosim));
            std::cout << std::fixed << std::setprecision(4) << cosim << "\n";
            setColor(7);
        }
    }
    std::cout << "\n";

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    // つ ◕_◕༽つ Boo  


    //////////////////////////////<<<<<<<<<<<<<<<<<<<<<<<    Precision, Recall, and F1-Score    >>>>>>>>>>>>>>>>>>>>//////////////////////////
    setColor(14);
    std::cout << "                                   === Precision, Recall, and F1-Score Evaluation ===\n";
    setColor(7);
    std::cout << "Evaluate the retrieval performance for a query term.\n";
    std::cout << "- Precision: Proportion of retrieved documents that are relevant.\n";
    std::cout << "- Recall: Proportion of relevant documents that were retrieved.\n";
    std::cout << "- F1-Score: Harmonic mean of Precision and Recall.\n\n";

    /////////////////////////////////////////////////////////   query term
    std::string query_term;
    bool valid_term = false;
    do {
        std::cout << "Enter a query term from the TF-IDF table above: ";
        std::getline(std::cin, query_term);
        query_term = toLower(query_term);
        if (terms.count(query_term)) {
            valid_term = true;
        }
        else {
            setColor(12);
            std::cout << "Error: '" << query_term << "' is not a valid term. Please choose a term from the TF-IDF table. \n";
            setColor(7);
        }
    } while (!valid_term);

    //////////////////////////////////// relevant documents
    std::set<int> relevant_docs;
    std::cout << "\nSelect relevant documents by entering their numbers (1 to " << N << "),\n";
    std::cout << "separated by spaces. Press Enter to finish. sone thing like this 1 2 for choosing document  1 and 2.\n";
    std::cout << "Available documents:\n";
    for (int i = 0; i < N; ++i) {
        std::cout << "  " << (i + 1) << ": " << names[i] << "\n";
    }
    std::cout << "Enter numbers: ";
    std::string input;
    std::getline(std::cin, input);
    std::stringstream ss(input);
    std::string number;
    while (ss >> number) {
        try {
            int idx = std::stoi(number);
            if (idx >= 1 && idx <= N) {
                relevant_docs.insert(idx - 1); ////////////////////////////// 1-based to 0-based
            }
            else {
                setColor(12);
                std::cout << "Warning: '" << number << "' is not a valid number (1 to " << N << "). Ignored.\n";
                setColor(7);
            }
        }
        catch (const std::exception&) {
            setColor(12);
            std::cout << "Warning: '" << number << "' is not a valid number. Ignored.\n";
            setColor(7);
        }
    }

    if (relevant_docs.empty()) {
        setColor(12);
        std::cout << "Warning: No valid relevant documents selected. Metrics will be zero.\n";
        setColor(7);
    }

    ///////////////////////////////////    Retrieve documents with non-zero TF-IDF for the query term
    std::set<int> retrieved_docs;
    for (int i = 0; i < N; ++i) {
        if (tfidf[i].count(query_term) && tfidf[i][query_term] > 0.0) {
            retrieved_docs.insert(i);
        }
    }

    // Calculate TP, FP, FN
    int tp = 0, fp = 0, fn = 0;
    for (int i : retrieved_docs) {
        if (relevant_docs.count(i)) {
            tp++; // Retrieved and relevant   \[T]/ PRAISE THE SUN
        }
        else {
            fp++; // Retrieved but not relevant   
        }
    }
    for (int i : relevant_docs) {
        if (!retrieved_docs.count(i)) {
            fn++; // Relevant but not retrieved
        }
    }

    // Compute column width for Retrieval Summary table
    size_t summary_col_width = 8; // Length of "Category"   :/
    std::string retrieved_str = retrieved_docs.empty() ? "None" : "";
    if (!retrieved_docs.empty()) {
        bool first = true;
        for (int i : retrieved_docs) {
            if (!first) retrieved_str += ", ";
            retrieved_str += names[i];
            first = false;
        }
    }
    std::string relevant_str = relevant_docs.empty() ? "None" : "";
    if (!relevant_docs.empty()) {
        bool first = true;
        for (int i : relevant_docs) {
            if (!first) relevant_str += ", ";
            relevant_str += names[i];
            first = false;
        }
    }
    summary_col_width = std::max(summary_col_width, std::string("Retrieved Documents: ").length() + retrieved_str.length());
    summary_col_width = std::max(summary_col_width, std::string("Relevant Documents: ").length() + relevant_str.length());
    summary_col_width += 2; // Add padding

    /////////////////////////////////////////////////////// Print Retrieval Summary table
    setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Soft blue
    std::cout << "\nRetrieval Summary for Query Term: " << query_term << "\n";
    std::cout << "+";
    std::cout << std::string(summary_col_width, '-') << "+\n";
    std::cout << "| " << std::left << std::setw(summary_col_width - 1) << "Category" << "|\n";
    std::cout << "+";
    std::cout << std::string(summary_col_width, '-') << "+\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Light gray
    std::cout << "| " << std::left << std::setw(summary_col_width - 1) << ("Retrieved Documents: " + retrieved_str) << "|\n";
    std::cout << "| " << std::left << std::setw(summary_col_width - 1) << ("Relevant Documents: " + relevant_str) << "|\n";
    setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Soft blue
    std::cout << "+";
    std::cout << std::string(summary_col_width, '-') << "+\n";
    setColor(7); // Default white

    // Compute column widths for Evaluation Metrics table
    size_t metric_col_width = 9; // Length of "Precision"
    size_t value_col_width = 6; // Length of "1.0000"
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(4) << (tp + fp > 0 ? static_cast<double>(tp) / (tp + fp) : 0.0);
        value_col_width = std::max(value_col_width, ss.str().length());
    }
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(4) << (tp + fn > 0 ? static_cast<double>(tp) / (tp + fn) : 0.0);
        value_col_width = std::max(value_col_width, ss.str().length());
    }
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(4) << ((tp + fp > 0 && tp + fn > 0) ? 2.0 * (static_cast<double>(tp) / (tp + fp)) * (static_cast<double>(tp) / (tp + fn)) / (static_cast<double>(tp) / (tp + fp) + static_cast<double>(tp) / (tp + fn)) : 0.0);
        value_col_width = std::max(value_col_width, ss.str().length());
    }
    metric_col_width += 2; // Add padding
    value_col_width += 2; //////////  ^

   
    double precision = (tp + fp) > 0 ? static_cast<double>(tp) / (tp + fp) : 0.0;
    double recall = (tp + fn) > 0 ? static_cast<double>(tp) / (tp + fn) : 0.0;
    double f1_score = (precision + recall) > 0 ? 2.0 * (precision * recall) / (precision + recall) : 0.0;

    ////////////////////////////////////////////////////////////// Print Evaluation
    setColor(14); // Yellow
    std::cout << "\nEvaluation Metrics:\n";
    setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Soft blue
    std::cout << "+";
    std::cout << std::string(metric_col_width, '-') << "+";
    std::cout << std::string(value_col_width, '-') << "+\n";
    std::cout << "| " << std::left << std::setw(metric_col_width - 1) << "Metric" << "|";
    std::cout << std::left << std::setw(value_col_width) << "Value" << "|\n";
    std::cout << "+";
    std::cout << std::string(metric_col_width, '-') << "+";
    std::cout << std::string(value_col_width, '-') << "+\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Light gray
    std::cout << "| " << std::left << std::setw(metric_col_width - 1) << "Precision" << "|";
    std::cout << std::left << std::setw(value_col_width) << std::fixed << std::setprecision(4) << precision << "|\n";
    std::cout << "| " << std::left << std::setw(metric_col_width - 1) << "Recall" << "|";
    std::cout << std::left << std::setw(value_col_width) << std::fixed << std::setprecision(4) << recall << "|\n";
    std::cout << "| " << std::left << std::setw(metric_col_width - 1) << "F1-Score" << "|";
    std::cout << std::left << std::setw(value_col_width) << std::fixed << std::setprecision(4) << f1_score << "|\n";
    setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Soft blue
    std::cout << "+";
    std::cout << std::string(metric_col_width, '-') << "+";
    std::cout << std::string(value_col_width, '-') << "+\n";
    setColor(7); // Default white

    // Closing message
    setColor(14);
    std::cout << "\n                                               === Analysis Complete ===\n";
    setColor(7);
    std::cout << "Thank you for using this side project of me. please consider looking at my other works: https://github.com/iwenKastarito \n";

    return 0;
}

// haha i code 500 lines. what am i doing with my life