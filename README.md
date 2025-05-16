# TF-IDF
a side project for univercity that calculate TF-IDF Scores, Cosine Similarity and Query Analysis.



## What Does It Do?
- **Select Files**: Pick one or more `.txt` files using a Windows file dialog.
- **TF-IDF Scores**: Shows how important words are in each document.
- **Cosine Similarity**: Compares documents to see how similar they are.
- **Query Analysis**: Enter a term and mark relevant documents to get Precision, Recall, and F1-Score.

## How to Use It
1. **Build**: Compile the C++ code on a Windows machine (uses `<windows.h>` and `<commdlg.h>`).
   - Example: `g++ main.cpp -o tfidf_tool`
2. **Run**: Launch the executable (`TFIDF.exe`).
3. **Select Files**: Choose `.txt` or files in the pop-up dialog.
4. **View Results**: Check the TF-IDF table, cosine similarities, and query metrics.
5. **Enter a Query**: Type a term from the TF-IDF table and select relevant documents (e.g., `1 2` for docs 1 and 2).

## Requirements
- Windows OS (for file dialog and console colors).
- C++ compiler (e.g., g++ with MinGW or MSVC).
- Text files (`.txt` or `.md`) to analyze.