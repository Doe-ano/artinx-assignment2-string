// ============================================================================
// 作业 2：String 类的实现文件
//
// 目前本文件是空的：构建时链接阶段会报 "undefined reference to `String::...`"，
// 这是预期现象。请先到 include/my_string.h 中补好私有数据成员，再在这里实现
// 所有声明过的成员函数与运算符。
//
// 如果你想拆成多个 .cpp 文件，请同步修改根目录 CMakeLists.txt 中的
// STRING_SOURCES 列表。
//
// 实现清单（与 include/my_string.h 一一对应）：
//   [ ] String() / String(const char*) / 拷贝构造 / 移动构造 / 析构
//   [ ] 复制赋值 operator=(const String&) / 移动赋值 operator=(String&&)
//   [ ] operator+ / operator[]（含 const 版本）/ at（含 const 版本）
//   [ ] size / capacity
//   [ ] insert / push_back
//   [ ] c_str / operator const char*
//   [ ] swap
//   [ ] friend operator<< / operator>>
//
// 完成后按 docs/build-and-test.md 的步骤构建、运行测试并做 ASan/UBSan 检查。
// ============================================================================

#include "my_string.h"
#include <stdexcept>

// TODO: 在此实现 include/my_string.h 中声明的所有成员函数与运算符。
// ---------------构造/析构--------------------
// 默认构造函数：创建一个空字符串
// 初始容量为16，实际缓冲区大小为17
String::String() : data_(new char[17]), size_(0), capacity_(16) {
    data_[0] = '\0';
    }

// 带参构造函数：用C字符串str构造String对象
String::String(const char* str) : data_(nullptr), size_(0), capacity_(0) {
    // 将str从空指针转换为指向‘\0’的地址
    if (str == nullptr) {
        str = "";
    }

    // 手动计算字符串的长度
    while (str[size_] != '\0') {
        ++size_;
    }

    capacity_ = size_;
    data_ = new char[capacity_ + 1];

    // data_末尾有'\0'
    for (std::size_t i = 0; i <= size_; ++i) {
        data_[i] = str[i];
    }
}

// 拷贝构造函数：用已有的string对象创建一个新对象
String::String(const String& other) : data_(new char[other.capacity_ + 1]), size_(other.size_), capacity_(other.capacity_) {
    for (std::size_t i = 0; i <= size_; ++i) {
        data_[i] = other.data_[i];
    }
}

// 移动构造函数：把other置空，而非复制
String::String(String&& other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
}

// 析构函数
String::~String() {
    delete[] data_;
}

// ---------------赋值--------------------
// 复制赋值：先不释放旧内存，先准备缓冲区，成功后再替换旧缓冲区
String& String::operator=(const String& other) {
    // 判断是否为自赋值（如果不判断的话会先把字符串自己的内存释放了再从other拷贝，但这样的话other.data_是悬空指针）
    if (this == &other) {
        return *this;
    }

    // 如果other被移动过，可能data_为空指针，需要将其视为空串，即size_也应为0
    const std::size_t new_size = (other.data_ == nullptr) ? 0 : other.size_;
    const std::size_t new_capacity = (other.data_ == nullptr) ? 0 : other.capacity_;

    // 分配新内存，返回这块内存的首地址给new_data
    char* new_data = new char[new_capacity + 1];

    // 将other的内容拷贝到新分配的new_data里
    if (other.data_ == nullptr) {
        new_data[0] = '\0';
    }else {
        for (std::size_t i = 0; i <= new_size; ++i) {
            new_data[i] = other.data_[i];
        }
    }

    delete[] data_;
    data_ = new_data;
    size_ = new_size;
    capacity_ = new_capacity;

    return *this;
}

// 移动赋值：释放当前缓冲区（没有new没有内存分配，整个过程是安全的），再接管other的缓冲区
String& String::operator=(String&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    delete[] data_;

    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;

    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;

    return *this;
}

// ---------------拼接--------------------
String String::operator+(const String& other) const {
    String result;

    for (std::size_t i = 0; i < size_; ++i) {
        result.push_back(data_[i]);     // 将原字符串逐字符加上去
    }

    for (std::size_t i = 0; i < other.size_; ++i) {
        result.push_back(other.data_[i]);   // 将other逐字符加入
    }

    return result;
}

// 在字符串末尾追加一个字符
void String::push_back(char ch) {
    if (data_ != nullptr && size_ < capacity_) {
        data_[size_] = ch;
        ++size_;
        data_[size_] = '\0';
        return;
    }

    // 容量不足，或对象此前被移动过
    std::size_t new_capacity = capacity_;
    if (new_capacity < 16) {
        new_capacity = 16;
    }else{
        new_capacity *= 2;
    }

    char* new_data = new char[new_capacity + 1];
    for (std::size_t i = 0; i < size_; ++i) {
        new_data[i] = (data_ == nullptr) ? '\0' : data_[i];
    }

    new_data[size_] = ch;
    new_data[size_ + 1] = '\0';

    delete[] data_;
    data_ = new_data;
    ++size_;
    capacity_ = new_capacity;
}

// ---------------下标访问--------------------
char& String::operator[](std::size_t index) noexcept {
    return data_[index];
}

// const对象不能调用非const成员函数，会报错，因此需要另外写const版
const char& String::operator[](std::size_t index) const noexcept {
    return data_[index];
}

// ---------------下标访问（带边界检查）--------------------
char& String::at(std::size_t index) {
    if (index >= size_) {
        throw std::out_of_range("String::at: index out of range");
    }
    return data_[index];
}

const char& String::at(std::size_t index) const {
    if (index >= size_) {
        throw std::out_of_range("String::at: index out of range");
    }
    return data_[index];
}

// ---------------长度/容量--------------------
std::size_t String::size() const noexcept {
    return size_;
}

std::size_t String::capacity() const noexcept {
    return capacity_;
}

// ---------------插入/追加--------------------
void String::insert(std::size_t pos, const String& str) {
    if (pos > size_){
        throw std::out_of_range("String::insert: position out of range");
    }

    // 插入为空字符串：直接结束
    if (str.size_ == 0) {
        return;
    }

    const std::size_t new_size = size_ + str.size_;
    std::size_t new_capacity = capacity_;

    if (new_capacity < new_size){
        new_capacity = new_size;
    }

    char* new_data = new char[new_capacity + 1];

    // 先复制插入之前的部分
    for (std::size_t i = 0; i < pos; ++i) {
        new_data[i] = data_[i];
    }

    // 再复制要插入的字符串
    for (std::size_t i = 0; i < str.size_; ++i) {
        new_data[pos + i] = str.data_[i];
    }

    // 最后复制原来字符串的剩余部分
    for (std::size_t i = pos; i < size_; ++i){
        new_data[str.size_ + i] = data_[i];
    }

    new_data[new_size] = '\0';

    delete[] data_;
    data_ = new_data;
    size_ = new_size;
    capacity_ = new_capacity;
}

// ---------------转换为C字符串--------------------
const char* String::c_str() const noexcept {
    if (data_ == nullptr) {
        return "";
    }
    return data_;
}

// 隐式转换为C字符串
String::operator const char*() const noexcept {
    return c_str();
}

// ---------------交换全部内容--------------------
void String::swap(String& other) noexcept {
    if (this == &other) {
        return;
    }

    char* temp_data = data_;
    data_ = other.data_;
    other.data_ = temp_data;

    std::size_t temp_size = size_;
    size_ = other.size_;
    other.size_ = temp_size;

    std::size_t temp_capacity = capacity_;
    capacity_ = other.capacity_;
    other.capacity_ = temp_capacity;
}

// ---------------流操作--------------------
#include <istream>
#include <locale>
#include <ostream>
#include <streambuf>
#include <utility>

// 按字符串长度输出内容，包括可能存在的'\0'
std::ostream& operator<<(std::ostream& os, const String& str) {
    if (str.data_ != nullptr && str.size_ > 0) {
        os.write(str.data_, static_cast<std::streamsize>(str.size_));
    }
    return os;
}

// 跳过开头空白，读取一个不含空白的字符串
std::istream& operator>>(std::istream& is, String& str) {
    std::istream::sentry sentry(is);   // 在流输入里创建一个sentry对象，用来做输入前的准备工作
    if (!sentry) {
        return is;                     // 流状态不对，直接返回
    }

    String temp;
    bool got_character = false;
    std::streambuf* buffer = is.rdbuf();   // 获取输入流底层的缓冲区指针，用来直接操作流的数据源
    // 获取当前输入流所使用的字符分类规则，用来判断哪些字符是空白、哪些是字母数字等
    const std::ctype<char>& char_type = std::use_facet<std::ctype<char>>(is.getloc());

    // 逐字符读取流，并判断是否到达文件末尾
    for (;;) {
        const std::istream::int_type next = buffer->sgetc();    // 看当前字符

        // 如果已经读到末尾
        if (std::istream::traits_type::eq_int_type(next, std::istream::traits_type::eof())) {
            is.setstate(std::ios_base::eofbit);                 // 标记流为eofbit（已到文件末尾）
            break;
        }

        // 把上面看到的字符转为char
        const char ch = std::istream::traits_type::to_char_type(next);
        // 判断ch是不是空白字符（空格、制表、换行等），如果是，跳出循环，表示该单词读完了
        if (char_type.is(std::ctype_base::space, ch)) {
            break;
        }

        buffer->sbumpc();           // stream bump character，取走当前字符并让位置前进一格
        temp.push_back(ch);
        got_character = true;
    }

    // 根据是否读到字符决定成功赋值还是标记失败
    if (got_character) {
        str = std::move(temp);      // temp马上会被销毁，直接移动赋值更快（str = temp是深拷贝，没必要）
    }else {
        is.setstate(std::ios_base::failbit);
    }

    return is;
}