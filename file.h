struct file {
  enum { FD_NONE, FD_PIPE, FD_INODE } type;
  int ref; // reference count
  char readable;
  char writable;
  struct pipe *pipe;
  struct inode *ip;
  uint off;
};


// in-memory copy of an inode
struct inode {
  uint dev;           // Device number
  uint inum;          // Inode number
  int ref;            // Reference count
                      // ref是此inode被引用的次数，如果ref为0，就说明没有进程使用这个inode了，
                      // 内核就会从内存中丢弃这个inode。iget和iput函数引用和释放inode，并修改引用计数
  struct sleeplock lock; // protects everything below here
  int valid;          // inode has been read from disk?

  short type;         // copy of disk inode
  short major;
  short minor;
  short nlink;
  uint size;
  uint addrs[NDIRECT+1]; // 数组的下标表示逻辑块号，数组元素则是对应的物理块号。
                         // 该inode对应的数据的block块的位置. 前NDIRECT个block直接保存用户数据，
                         // 最后一个block里又保存了NDIRECT个block块的块号， 相当于两层了，所以
                         // 文件最大为:512B * 12 + 512B * 12 = 12KB.

};

// table mapping major device number to
// device functions
struct devsw {
  int (*read)(struct inode*, char*, int);
  int (*write)(struct inode*, char*, int);
};

extern struct devsw devsw[];

#define CONSOLE 1
