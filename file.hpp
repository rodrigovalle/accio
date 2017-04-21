class File {
 public:
  File() = delete;
  File(const File&) = delete;
  File(File&&);
  ~File();

  File& operator=(const File&) = delete;
  File& operator=(File&&);

  static File open_r(std::string& file);
  static File create_w(std::string& file);

 private:
  static File open(std::string& file, int flags);
  File(int fd);
  int fd;
};
