#include <iostream>
#include <cstdio>
#include <utility>
#include <vector>
#include <sstream>
#include <cstring>

#define Unknown_Command "Unknown Command.\n"
#define Param_Error "Parameter Error.\n"
#define NoSuchDirectory "DIR NOT FOUND.\n"
#define NoSuchFile "FILE NOT FOUND\n"
#define MAX_FILE_SIZE 10000

// BPB
// BS_jmpBOOT 短跳转指令
// BS_OEMName 厂商名
int BytesPerSec; //每扇区字节数量 默认为512
int SecPerCluster; //每簇扇区数量 默认为1
int BytesPerCluster; //每簇字节数量 not BPB
int ReservedSecCount; //Boot记录占用多少扇区，默认为Master Boot Record保留1个扇区数量
int NumOfFats; //Fat表数量，默认2个FAT表
int RootEntCnt; //根目录区文件最大数量 默认224
// BPB_TotalSec16 扇区总数
// BPB_Media 介质描述符
int FatSz16; //一个FAT表所占用的扇区 默认9
// BPB_SecPerTrk 每磁道扇区数量
// BPB_NumHeads 磁头数量
// BPB_HiddSec 隐藏扇区数量
// BPB_TotalSec32 32位扇区总数,若Sec16为0，才使用这个值
// BS_DrvNum 驱动器号
// BS_Reserved1 保留
// BS_BootSig 扩展引导标记
// BS_VolID 卷序列号
// BS_VolLab 卷标
// BS_FilSysType 文件系统类型
// BS_BootCode 引导代码
// 结束标识 0xAA55
// address area
int fatBase; //fat表基址
int fileRootBase; //根目录基址
int dataBase; //数据基址
#pragma pack(1)

using namespace std;

extern "C" {
void my_print(const char *, int);
void change_to_red();
void back_to_default();
}

void myPrint(const char *s) // 做了一层封装
{ my_print(s, strlen(s)); }

//树状链表存放
class Node
{
    //node里面多放点东西 根节点默认为目录节点，前驱为自己。
    string path; //路径
    string name; //目录或文件名称 path+name=绝对路径
    Node *pre{}; //前继节点
    vector<Node *> suc; //后继节点
    bool isFile = false; //判断文件或目录
    bool isVal = true; //值或非值 false则为.或..
    int directory_count = 0; //下级目录个数
    int file_count = 0; //目录下级文件个数
    uint32_t size{}; //文件大小
    char *content = new char[MAX_FILE_SIZE]; //文件内容
public:
    Node() = default;//默认是目录，下级目录和文件为. ..
    Node(string name, bool isVal) //一般非值
    {
        this->name = move(name);
        this->isVal = isVal;
    }

    Node(string name, uint32_t size, bool isFile, string path) // 用于文件
    {
        this->name = move(name);
        this->path = move(path);
        this->isFile = isFile;
        this->size = size;
    }

    void setPath(string path)
    { this->path = move(path); }

    void setName(string name)
    { this->name = move(name); }

    void setPre(Node *pre)
    { this->pre = pre; }

    //保持树状双链表
    void addChild(Node *succ)
    {
        this->suc.push_back(succ);
        succ->pre = this;
    }

    void addFileChild(Node *succ)
    {
        this->addChild(succ);
        this->file_count++;
    }

    void addDirChild(Node *succ)
    {
        this->addChild(succ);
        this->directory_count++;
        succ->addChild(new Node(".", false));
        succ->addChild(new Node("..", false));
    }

    string getPath()
    { return this->path; }

    string getName()
    { return this->name; }

    Node *getPre()
    { return this->pre; }

    vector<Node *> getSuc()
    { return this->suc; }

    uint32_t getSize() const
    { return this->size; }

    char *getContent()
    { return this->content; }

    int get_dir_count() const
    { return this->directory_count; }

    int get_file_count() const
    { return this->file_count; }

    bool CheckIsFile() const
    { return this->isFile; }

    bool CheckIsVal() const
    { return this->isVal; }
};

class MBR
{
    uint16_t BPB_BytesPerSector{};//每扇区字节数 11-12（默认512）
    uint8_t BPB_SectorsPerCluster{};//每簇扇区数 13-13（默认1）
    uint16_t BPB_ReservedSectorCount{};//MBR占用的扇区数 14-15（默认1）
    uint8_t BPB_NumFATs{};//FAT表的数量 16-16 （默认2）
    uint16_t BPB_RootEntryCount{};//根目录区文件最大数量 17-18 （默认224）
    __attribute__((unused)) uint16_t BPB_TotalSector16{};//限于16bit的扇区总数 19-20 （默认2880）
    __attribute__((unused)) uint8_t BPB_Media{};//介质描述符 21-21 （默认0xF0）
    uint16_t BPB_FATSector_z_16{};//一个FAT表所占扇区 22-23 （默认9）
    __attribute__((unused)) uint16_t BPB_SectorPerTrack{};//每磁道扇区数量 24-25
    __attribute__((unused)) uint16_t BPB_NumHeads{};//磁头数 26-27
    __attribute__((unused)) uint32_t BPB_HiddenSector{};//隐藏扇区数量 28-31
    __attribute__((unused)) uint32_t BPB_TotalSector32{};//16位存不下置0在这里存 32-35
public:
    MBR() = default;

    void init(FILE *img)
    {
        fseek(img, 11, SEEK_SET);//重新定义文件指针
        fread(this, 1, 25, img);//以一个字节为单位基准，读取从第十一字节开始的25个字节

        BytesPerSec = this->BPB_BytesPerSector;
        SecPerCluster = this->BPB_SectorsPerCluster;
        BytesPerCluster = BytesPerSec * SecPerCluster;
        ReservedSecCount = this->BPB_ReservedSectorCount;
        NumOfFats = this->BPB_NumFATs;
        RootEntCnt = this->BPB_RootEntryCount;
        FatSz16 = this->BPB_FATSector_z_16;
        //基址
        fatBase = ReservedSecCount * BytesPerSec;
        fileRootBase = fatBase + NumOfFats * FatSz16 * BytesPerSec;
        dataBase =
                fileRootBase + (RootEntCnt * 32 + BytesPerSec - 1) / BytesPerSec * BytesPerSec;//簇是数据区的组织方式而非其他区域。
    }
};

class RootEntry
{
    char fileName[8]{};//0-7
    char extendName[3]{};//8-10
    uint8_t attribute{};//11-11
    __attribute__((unused)) char reserved[10]{};//12-21
    __attribute__((unused)) uint16_t createTime{};//22-23
    __attribute__((unused)) uint16_t createDate{};//24-25
    uint16_t firstCluster{};//26-27
    uint32_t fileSize{};//28-31
public:
    RootEntry() = default;

    bool isEmpty()
    { return this->fileName[0] == '\0'; }
    bool isInvalid()
    {
        bool check = false;
        for (char k: this->fileName)
        {
            if (!(isupper(k) || isdigit(k) || k == ' '))
                check = true;
        }
        for (char k: this->extendName)
        {
            if (!(isupper(k) || isdigit(k) || k == ' '))
                check = true;
        }
        return check;
    }
    bool isFile() const //0x10是文件属性字段中用于表示目录的标志位
    { return (this->attribute & 0x10) == 0; }

    // 初始化树状链表
    void init(FILE *img, Node *root)
    {
        int base = fileRootBase; //base会发生变化
        char info[12];
        for (int i = 0; i < RootEntCnt; i++)
        {
            fseek(img, base, SEEK_SET);
            fread(this, 1, 32, img);
            base = base + 32;
            if (!isEmpty() && !isInvalid())
            {
                if (isFile())
                {
                    // 获取文件名
                    fetchFileName(info);
                    // 创建文件节点
                    Node *child = new Node(info, this->fileSize, true, root->getPath());//path+name
                    // 添加到当前根节点
                    root->addFileChild(child);
                    // 获取文件内容
                    getFileContent(img, this->firstCluster, child);//递归
                } else
                {
                    // 获取目录名
                    fetchDirName(info);
                    // 创建目录节点
                    Node *child = new Node();
                    child->setName(info);
                    child->setPath(root->getPath());
                    // 添加到当前根节点
                    root->addDirChild(child);
                    // 将新的子目录拓展到根节点
                    extendChild(img, this->firstCluster, child);//递归
                }
            }
        }
    }

    void fetchFileName(char info[12])
    {
        int tmp = 0;
        for (int i = 0; i < 8 && fileName[i] != ' '; i++)
            info[tmp++] = fileName[i];
        info[tmp++] = '.';
        for (int i = 0; i < 3 && fileName[i] != ' '; i++)
            info[tmp++] = extendName[i];
        info[tmp] = '\0';
    }

    void fetchDirName(char info[12])
    {
        int tmp = 0;
        for (char ch: fileName)
        {
            if (ch != ' ')
                info[tmp++] = ch;
        }
        info[tmp] = '\0';
    }

    static void getFileContent(FILE *img, int cluster, Node *root)
    {
        if (cluster <= 1)
            return;
        int curCluster = cluster;
        int value = 0;
        char *toFilled = root->getContent();

        while (value < 0xFF8)
        {
            value = getValue(img, curCluster);// 获取下一个簇号
            if (value == 0xFF7)
            {
                myPrint("broken cluster\n");
                break;
            }
            char *buffer = (char *) malloc(BytesPerCluster);
            int start = dataBase + (curCluster - 2) * BytesPerCluster;
            fseek(img, start, SEEK_SET);
            fread(buffer, 1, BytesPerCluster, img);
            for (int i = 0; i < BytesPerCluster; i++)
            {
                *toFilled = buffer[i];
                toFilled++;
            }
            free(buffer);
            curCluster = value;
        }
    }

    void extendChild(FILE *img, int cluster, Node *root)
    {
        if(cluster <= 1)
            return;
        int curCluster = cluster;
        int value = 0;
        while (value < 0xFF8)
        {
            value = getValue(img, curCluster);
            if (value == 0xFF7)
            {
                myPrint("broken cluster\n");
                break;
            }
            int start = dataBase + (curCluster - 2) * BytesPerCluster;
            for (int i = 0; i < BytesPerCluster; i += 32)
            {
                auto *rootEntry = new RootEntry();
                fseek(img, start + i, SEEK_SET);
                fread(rootEntry, 1, 32, img);
                if (rootEntry->isEmpty() || rootEntry->isInvalid())
                    continue;
                char tmpName[12];
                if (rootEntry->isFile())
                {
                    rootEntry->fetchFileName(tmpName);
                    Node *child = new Node(tmpName, rootEntry->fileSize, true, root->getPath() + root->getName() + "/");
                    root->addFileChild(child);
                    getFileContent(img, rootEntry->firstCluster, child);
                } else
                {
                    rootEntry->fetchDirName(tmpName);
                    Node *child = new Node();
                    child->setName(tmpName);
                    child->setPath(root->getPath() + root->getName() + "/");
                    root->addDirChild(child);
                    extendChild(img, rootEntry->firstCluster, child);
                }
            }
        }
    }

    static uint16_t processBytes(uint16_t bytes, int num)
    {
        if (num % 2 == 0)
            bytes = bytes << 4;
        return bytes >> 4;
    }

    static int getValue(FILE *img, int num)
    {
        int base = ReservedSecCount * BytesPerSec;
        int pos = base + num * 3 / 2; // 3/2是因为两个簇号占3个字节
        uint16_t bytes;
        fseek(img, pos, SEEK_SET);
        fread(&bytes, 1, 2, img);
        return processBytes(bytes, num);
    }
};

vector <string> breakPath(const string &path)
{
    char *s = new char[path.size() + 1];
    strcpy(s, path.c_str());
    char *p = strtok(s, "/");
    vector <string> units;
    while (p)
    {
        units.emplace_back(p);
        p = strtok(nullptr, "/");
    }
    return units;
}


Node *visit(Node *root, const vector <string> &units)
{
    Node *tmp = root;
    for (auto &unit: units)
    {
        if (unit == ".")
        {
            continue;
        } else if (unit == "..")
        {
            tmp = tmp->getPre();
        } else
        {
            bool notfound = true;
            vector < Node * > children = tmp->getSuc();
            for (int j = 0; j < children.size(); j++)
            {
                if (children[j]->getName() == unit)
                {
                    notfound = false;
                    tmp = children[j];
                    break;
                }
            }
            if (notfound)
                return nullptr; 
        }
    }
    return tmp;
}

void printCAT(Node *root, const string &path)
{
    vector <string> units = breakPath(path);
    Node *finalNode= visit(root, units);
    if (finalNode== nullptr || !finalNode->CheckIsFile())
    {
        myPrint(NoSuchFile);
        return;
    }
    myPrint(finalNode->getContent());
}

void printLS_Impl(Node *root)
{
    string path = root->getPath() + root->getName();
    if (path[path.size() - 1] != '/')
        path += "/";
    path += ":\n";
    myPrint(path.c_str());
    for (int i = 0; i < root->getSuc().size(); i++) // 打印直接子目录和文件
    {
        Node *item = root->getSuc()[i];
        if (!item->CheckIsVal() || !item->CheckIsFile())
        {
            change_to_red();
            myPrint(item->getName().c_str());
            back_to_default();
        } else
            myPrint(item->getName().c_str());
        myPrint("  ");
    }
    myPrint("\n");
    for (int i = 0; i < root->getSuc().size(); i++) // 递归打印子目录下的内容
    {
        Node *item = root->getSuc()[i];
        if (item->CheckIsVal() && !item->CheckIsFile())
            printLS_Impl(item);
    }
}

void printLS(Node *root, const string &path)
{
    vector <string> units = breakPath(path);
    Node *finalNode= visit(root, units);
    if (finalNode== nullptr || finalNode->CheckIsFile())
    {
        myPrint(NoSuchDirectory);
        return;
    }
    printLS_Impl(finalNode);
}

void printLS_L_Impl(Node *root)
{
    string path = root->getPath() + root->getName();
    if (path[path.size() - 1] != '/')
        path = path + "/";
    path = path + " " + to_string(root->get_dir_count()) + " " + to_string(root->get_file_count()) + ":\n";
    myPrint(path.c_str());
    for (int i = 0; i < root->getSuc().size(); i++)
    {
        Node *item = root->getSuc()[i];
        if (!item->CheckIsVal())
        {
            change_to_red();
            myPrint(item->getName().c_str());
            back_to_default();
        } else if (!item->CheckIsFile())
        {
            change_to_red();
            myPrint(item->getName().c_str());
            back_to_default();
            myPrint("  ");
            myPrint(to_string(item->get_dir_count()).c_str());
            myPrint(" ");
            myPrint(to_string(item->get_file_count()).c_str());
        } else
        {
            myPrint(item->getName().c_str());
            myPrint("  ");
            myPrint(to_string(item->getSize()).c_str());
        }
        myPrint("\n");
    }
    myPrint("\n");
    for (int i = 0; i < root->getSuc().size(); i++)
    {
        Node *item = root->getSuc()[i];
        if (item->CheckIsVal() && !item->CheckIsFile())
            printLS_L_Impl(item);
    }
}

void printLS_L(Node *root, const string &path)
{
    vector <string> units = breakPath(path);
    Node *finalNode= visit(root, units);
    if (finalNode== nullptr || finalNode->CheckIsFile())
    {
        myPrint(NoSuchDirectory);
        return;
    }
    printLS_L_Impl(finalNode);
}

int main()
{
    FILE *img;
    img = fopen("lab2.img", "rb");

    Node *root = new Node();
    root->setPath("/");
    root->setName("");
    root->setPre(root);

    MBR *mbr = new MBR();
    mbr->init(img);

    auto *rootEntry = new RootEntry();
    rootEntry->init(img, root);

    while (true)
    {
        myPrint(">");
        string str;
        getline(cin, str);
        if (str.empty())
            continue;
        istringstream string_split(str);
        vector <string> words;
        string tempWord;
        while (string_split >> tempWord)
            words.push_back(tempWord); 

        if (words[0] == "exit")
            break;
        if (words[0] != "ls" && words[0] != "cat")
            myPrint(Unknown_Command);
        if (words[0] == "ls")
        {
            // 三个参数：ls、路径和-l  
            int check[words.size()];//1代表指令 2代表参数 3代表路径
            bool error = false;
            for (int i = 0; i < words.size(); i++)
            {
                if (words[i] == "ls")
                {
                    check[i] = 1; //1代表指令
                } else if (words[i][0] == '-')
                {
                    if (words[i].size() <= 1)
                    { error = true; }
                    int j = 1;
                    while (j < words[i].length())
                    {
                        if (words[i][j] != 'l') // -l之后出现l之外的其它均为参数错误
                            error = true;
                        j++;
                    }
                    check[i] = 2; //2代表参数
                } else
                {
                    check[i] = 3; //3代表路径
                }
            }
            if (error)
            {
                myPrint(Param_Error);
            } else
            {
                bool isL = false;
                int path_num = 0;
                for (int i = 0; i < words.size(); i++)
                {
                    if (check[i] == 2)
                        isL = true;
                    if (check[i] == 3)
                        path_num++;
                }
                if (path_num > 1) // 多于一个路径参数
                {
                    myPrint(Param_Error);
                } else
                {
                    int path_set = 0;
                    for (int i = 0; i < words.size(); i++)
                    {
                        if (check[i] == 3)
                            path_set = i;
                    }
                    if (isL)
                    {
                        string tempPath = path_set ? words[path_set] : "";
                        printLS_L(root, tempPath);
                    } else
                    {
                        string tempPath = path_set ? words[path_set] : "";
                        printLS(root, tempPath);
                    }
                }
            }
        }
        if (words[0] == "cat")
        {
            if (words.size() == 2)
                printCAT(root, words[1]);
            else
                myPrint(Param_Error);
        }
    }
    return 0;
}