// ================================================================
// fstream.sl — File I/O Library for Silo
//
// Provides C++-style file stream operations. All core operations
// are backed by native C++ fstream for full speed.
//
// USAGE — Global functions (fastest, recommended):
//
//   string content = readFile("data.txt");
//   writeFile("output.txt", "hello world");
//   appendFile("log.txt", "\nnew entry");
//   bool ok = fileExists("data.txt");
//   bool deleted = deleteFile("old.txt");
//   bool moved = renameFile("old.txt", "new.txt");
//   int sz = fileSize("data.txt");
//
// USAGE — FileStream class (OOP style):
//
//   FileStream fs("data.txt");
//   string content = fs.read();
//   fs.write("hello");
//   fs.append(" world");
//   if (fs.exists()) { fs.remove(); }
//
// NOTE: The FileStream class stores only the filename. Each
// method opens/closes the file. For high-volume I/O, use the
// global functions directly in a loop.
// ================================================================

class FileStream {

    string _path = "";

    // ── constructor ───────────────────────────────────────────────
    constructor FileStream(string path) {
        self._path = path;
    }

    // read()
    // Reads the entire file and returns it as a string.
    // Throws an error if the file cannot be opened.
    string read() {
        return readFile(self._path);
    }

    // write(content)
    // Overwrites the file with content.
    // Creates the file if it does not exist.
    // Returns the content written.
    string write(string content) {
        return writeFile(self._path, content);
    }

    // append(content)
    // Appends content to the end of the file.
    // Creates the file if it does not exist.
    // Returns the content appended.
    string append(string content) {
        return appendFile(self._path, content);
    }

    // exists()
    // Returns 1 (true) if the file exists, 0 otherwise.
    bool exists() {
        return fileExists(self._path);
    }

    // remove()
    // Deletes the file.
    // Returns 1 (true) on success, 0 otherwise.
    bool remove() {
        return deleteFile(self._path);
    }

    // rename(newPath)
    // Renames/moves the file to newPath.
    // Returns 1 (true) on success, 0 otherwise.
    bool rename(string newPath) {
        return renameFile(self._path, newPath);
    }

    // size()
    // Returns the file size in bytes.
    // Returns 0 if the file does not exist.
    int size() {
        return fileSize(self._path);
    }

    // path()
    // Returns the file path associated with this stream.
    string path() {
        return self._path;
    }

}
