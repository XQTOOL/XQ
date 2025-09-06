#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <limits>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <cmath>

namespace fs = std::filesystem;

// ===== 全局常量定义 =====
const fs::path UNPACK_DAT_PATH = "/storage/emulated/0/Download/XQ三合一/解包/dat";
const fs::path UNPACK_UEXP_PATH = "/storage/emulated/0/Download/XQ三合一/解包/uexp";
const fs::path MAIN_SCAN_DIR = "/storage/emulated/0/Download/XQ三合一/提取dat";
const fs::path OUTPUT_DIR = "/storage/emulated/0/Download/XQ三合一/特征码";
const fs::path SMALL_PACK_DIR = "/storage/emulated/0/Download/XQ三合一/小包";
std::string selected_pak_file; // 存储用户选择的pak文件路径

// ===== 工具函数声明 =====
bool path_exists(const fs::path& path);
bool select_pak_file(std::string& selected_file);
bool clear_directory_path(const fs::path& path);
bool create_directory_path(const fs::path& path);

// ===== 解包功能 =====
void unpack_dat();
void unpack_uexp();

// ===== 提取dat功能 =====
void extract_dat_files();

// ===== 特征码提取功能 =====
void extract_all_features();

// ===== 抓小包功能 =====
void grab_small_packs();

// ===== 主流程控制 =====
void run_full_automation();

// ===== 字体颜色功能 =====
size_t utf8CharLength(unsigned char c);
std::string gradientText(const std::string& text, int r1, int g1, int b1, int r2, int g2, int b2);
std::string coloredText(const std::string& text, int r, int g, int b);

// ===== 工具函数实现 =====
bool path_exists(const fs::path& path) {
    return fs::exists(path);
}

bool select_pak_file_from_directory(std::string& selected_file) {
    fs::path pak_dir = "/storage/emulated/0/Download/XQ三合一/pak";
    
    if (!path_exists(pak_dir)) {
        std::cout << coloredText("pak目录不存在: ", 255, 0, 0) << pak_dir.string() << std::endl;
        return false;
    }
    
    // 获取目录中的所有pak文件
    std::vector<fs::path> pak_files;
    try {
        for (const auto& entry : fs::directory_iterator(pak_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".pak") {
                pak_files.push_back(entry.path());
            }
        }
    } catch (const std::exception& e) {
        std::cout << coloredText("读取pak目录失败: ", 255, 0, 0) << e.what() << std::endl;
        return false;
    }
    
    if (pak_files.empty()) {
        std::cout << coloredText("pak目录中没有找到.pak文件", 255, 165, 0) << std::endl;
        return false;
    }
    
    // 显示文件列表供用户选择
    std::cout << gradientText("\n=== 可用的pak文件 ===", 0, 255, 255, 255, 0, 255) << std::endl;
    for (size_t i = 0; i < pak_files.size(); ++i) {
        std::cout << coloredText(std::to_string(i + 1) + ". ", 0, 255, 0) 
                  << gradientText(pak_files[i].filename().string(), 255, 215, 0, 255, 105, 180) << std::endl;
    }
    
    std::cout << coloredText("\n请选择pak文件序号 (1-", 0, 191, 255) 
              << coloredText(std::to_string(pak_files.size()), 255, 255, 0)
              << coloredText("): ", 0, 191, 255);
    size_t choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    if (choice < 1 || choice > pak_files.size()) {
        std::cout << coloredText("无效的选择", 255, 0, 0) << std::endl;
        return false;
    }
    
    selected_file = pak_files[choice - 1].string();
    std::cout << coloredText("已选择: ", 0, 255, 0) 
              << gradientText(selected_file, 0, 255, 255, 0, 191, 255) << std::endl;
    return true;
}

bool select_pak_file(std::string& selected_file) {
    // 简化的文件选择 - 在实际应用中应该实现文件选择对话框
    std::cout << coloredText("请选择pak文件路径: ", 0, 191, 255);
    std::getline(std::cin, selected_file);
    return !selected_file.empty() && path_exists(selected_file);
}

bool clear_directory_path(const fs::path& path) {
    try {
        if (path_exists(path)) {
            for (const auto& entry : fs::directory_iterator(path)) {
                fs::remove_all(entry.path());
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool create_directory_path(const fs::path& path) {
    try {
        return fs::create_directories(path);
    } catch (...) {
        return false;
    }
}

// ===== 字体颜色功能实现 =====
size_t utf8CharLength(unsigned char c) {
    if (c < 0x80) return 1;       // ASCII字符
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1; // 无效UTF-8头字节默认按单字节处理
}

std::string gradientText(const std::string& text, int r1, int g1, int b1, int r2, int g2, int b2) {
    std::string result;
    std::vector<size_t> char_positions;
    
    // 第一次遍历：记录所有UTF-8字符的起始位置
    size_t pos = 0;
    while (pos < text.length()) {
        char_positions.push_back(pos);
        pos += utf8CharLength(text[pos]);
    }
    const size_t char_count = char_positions.size();

    // 第二次遍历：应用渐变颜色
    for (size_t i = 0; i < char_count; ++i) {
        // 计算插值颜色
        float ratio = static_cast<float>(i) / (char_count - 1);
        int r = static_cast<int>(r1 + (r2 - r1) * ratio);
        int g = static_cast<int>(g1 + (g2 - g1) * ratio);
        int b = static_cast<int>(b1 + (b2 - b1) * ratio);

        // 提取完整字符
        const size_t start = char_positions[i];
        const size_t end = (i + 1 < char_count) ? char_positions[i+1] : text.length();
        const std::string character = text.substr(start, end - start);

        // 拼接ANSI代码
        result += "\033[38;2;" 
                + std::to_string(r) + ";" 
                + std::to_string(g) + ";" 
                + std::to_string(b) + "m" 
                + character;
    }
    result += "\033[0m"; // 重置颜色
    return result;
}

std::string coloredText(const std::string& text, int r, int g, int b) {
    return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m" + text + "\033[0m";
}

// ===== 非块解包功能 =====
void unpack_uexp() {
    std::cout << gradientText("🌀 启动非块解包工具 (.uexp)", 0, 255, 255, 0, 0, 255) << std::endl;

    // 检查解包工具是否存在
    if (!path_exists("./xiaoqi/Script/paks")) {
        std::cout << coloredText("未找到 paks 可执行文件", 255, 0, 0) << std::endl;
        return;
    }

    // 如果还没有选择pak文件，让用户选择
    if (selected_pak_file.empty()) {
        if (!select_pak_file_from_directory(selected_pak_file)) {
            return;
        }
    }

    // 清空目标目录
    if (path_exists(UNPACK_UEXP_PATH)) {
        std::cout << coloredText("正在清空解包目录...", 255, 165, 0) << std::endl;
        if (!clear_directory_path(UNPACK_UEXP_PATH)) {
            std::cout << coloredText("清空解包目录失败", 255, 0, 0) << std::endl;
            return;
        }
    } else {
        if (!create_directory_path(UNPACK_UEXP_PATH)) {
            std::cout << coloredText("创建解包目录失败", 255, 0, 0) << std::endl;
            return;
        }
    }

    std::cout << gradientText("⚡ uexp解包中...", 255, 0, 0, 255, 255, 0) << std::endl;
    
    // 执行解包命令
    std::string command1 = "./xiaoqi/Script/paks -a \"" + selected_pak_file + "\" \"" + UNPACK_UEXP_PATH.string() + "\"";
    
    int result1 = system(command1.c_str());
    
    if (result1 == 0) {
        std::cout << coloredText("解包完成! 文件已保存到 ", 0, 255, 0) 
                  << gradientText(UNPACK_UEXP_PATH.string(), 0, 255, 255, 0, 191, 255) << std::endl;
    } else {
        std::cout << coloredText("解包过程中出现错误", 255, 0, 0) << std::endl;
    }
}

// ===== 块解包功能 =====
void unpack_dat() {
    std::cout << gradientText("⚡ 启动块解包工具 (.dat)", 255, 0, 0, 255, 255, 0) << std::endl;
    
    // 检查 quickbms 是否存在
    if (!path_exists("./xiaoqi/Script/quickbms")) {
        std::cout << coloredText("当前目录未找到 quickbms 可执行文件", 255, 0, 0) << std::endl;
        return;
    }

    // 如果还没有选择pak文件，让用户选择
    if (selected_pak_file.empty()) {
        if (!select_pak_file_from_directory(selected_pak_file)) {
            return;
        }
    }
    
    // 检查解包脚本是否存在
    std::string script_path = "xiaoqi/Script/解包.bms";
    if (!path_exists(script_path)) {
        std::cout << coloredText("未找到'解包.bms'，请输入完整路径", 255, 165, 0) << std::endl;
        std::cout << coloredText("请输入脚本完整路径: ", 0, 191, 255);
        std::getline(std::cin, script_path);
        if (!path_exists(script_path)) {
            std::cout << coloredText("指定的脚本文件不存在", 255, 0, 0) << std::endl;
            return;
        }
    }
    
    // 清空目标目录
    if (path_exists(UNPACK_DAT_PATH)) {
        std::cout << coloredText("正在清空解包目录...", 255, 165, 0) << std::endl;
        if (!clear_directory_path(UNPACK_DAT_PATH)) {
            std::cout << coloredText("清空解包目录失败", 255, 0, 0) << std::endl;
            return;
        }
    } else {
        if (!create_directory_path(UNPACK_DAT_PATH)) {
            std::cout << coloredText("创建解包目录失败", 255, 0, 0) << std::endl;
            return;
        }
    }
    
    std::cout << gradientText("🌀 正在解包中...", 0, 255, 255, 0, 0, 255) << std::endl;
    
    // 执行解包命令
    std::string command = "qemu-i386 ./xiaoqi/Script/quickbms \"" + script_path + "\" \"" + selected_pak_file + "\" \"" + UNPACK_DAT_PATH.string() + "\"";
    int result = system(command.c_str());
    
    if (result == 0) {
        std::cout << coloredText("解包完成! 文件已保存到 ", 0, 255, 0) 
                  << gradientText(UNPACK_DAT_PATH.string(), 0, 255, 255, 0, 191, 255) << std::endl;
    } else {
        std::cout << coloredText("解包过程中出现错误", 255, 0, 0) << std::endl;
    }
}

// ===== 提取dat功能实现 =====
std::vector<std::string> file_contains_string(const fs::path& file_path, const std::vector<std::string>& search_strings) {
    std::vector<std::string> matched_strings;
    try {
        std::ifstream file(file_path, std::ios::binary);
        if (!file) return matched_strings;
        
        std::string content((std::istreambuf_iterator<char>(file)), 
                          std::istreambuf_iterator<char>());
        
        for (const auto& s : search_strings) {
            if (content.find(s) != std::string::npos) {
                matched_strings.push_back(s);
            }
        }
    } catch (...) {}
    return matched_strings;
}

void extract_to_folder(const fs::path& src_file, const fs::path& dest_folder) {
    try {
        // 清空目标文件夹（仅在提取新文件时执行）
        if (path_exists(dest_folder)) {
            for (const auto& entry : fs::directory_iterator(dest_folder)) {
                if (entry.is_regular_file()) {
                    fs::remove(entry.path());
                }
            }
        }
        
        // 复制新文件
        fs::copy_file(src_file, dest_folder / src_file.filename(), fs::copy_options::overwrite_existing);
        
        std::cout << coloredText("提取文件: ", 0, 255, 0) 
                  << gradientText(src_file.filename().string(), 255, 215, 0, 255, 105, 180)
                  << coloredText(" 到 [", 0, 191, 255)
                  << gradientText(dest_folder.filename().string(), 0, 255, 255, 0, 191, 255)
                  << coloredText("]", 0, 191, 255) << std::endl;
    } catch (...) {}
}

void extract_matching_files(const fs::path& input_dir, 
                          const fs::path& output_dir,
                          const std::unordered_map<std::string, std::string>& string_to_folder) {
    try {
        fs::create_directories(output_dir);
        
        // 记录哪些文件夹需要清理（只有这些文件夹会被清空旧文件）
        std::unordered_set<std::string> folders_to_clean;
        
        // 遍历输入文件
        for (const auto& entry : fs::recursive_directory_iterator(input_dir)) {
            if (!entry.is_regular_file()) continue;
            
            auto file_path = entry.path();
            auto matched_strings = file_contains_string(file_path, [&]{
                std::vector<std::string> keys;
                for (const auto& [s, _] : string_to_folder) keys.push_back(s);
                return keys;
            }());
            
            // 处理匹配的文件
            for (const auto& s : matched_strings) {
                auto folder_name = string_to_folder.at(s);
                fs::path dest_folder = output_dir / folder_name;
                fs::create_directories(dest_folder);
                
                // 标记需要清理的文件夹
                folders_to_clean.insert(folder_name);
                
                // 提取文件（会自动清理旧文件）
                extract_to_folder(file_path, dest_folder);
            }
        }
        
        // 输出未被更新的文件夹
        for (const auto& [_, folder_name] : string_to_folder) {
            if (folders_to_clean.find(folder_name) == folders_to_clean.end()) {
                std::cout << coloredText("文件夹 [", 255, 165, 0) 
                          << gradientText(folder_name, 255, 215, 0, 255, 105, 180)
                          << coloredText("] 未更新，保留旧文件", 255, 165, 0) << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << coloredText("错误: ", 255, 0, 0) << e.what() << std::endl;
    }
}

void extract_dat_files() {
    std::cout << gradientText("🚀 开始提取dat文件", 255, 0, 0, 255, 255, 0) << std::endl;
    
    // ===== 配置区域 =====
    std::unordered_map<std::string, std::string> STRING_TO_FOLDER = {
        {"/Game/UMG/UI_BP/Research/Research_MVP_UIBP.Research_MVP_UIBP_C", "淘汰播报"},
        {"/Game/UMG/_LobbySystem/_Lobby_Base/PartnerAI/AvatarFrame/UIBP/PartnerAI_AvatarFrame_HAT_UIBP.PartnerAI_AvatarFrame_HAT_UIBP_C", "头像框"},
        {"/Game/Arts_PlayerBluePrints/Character_Show/Emote/Hi_EmoteHandle_BP.Hi_EmoteHandle_BP_C", "大厅动作"},
        {"Play_UI_Equip_Cloth", "美化"},
        {"/Game/Mod/Escape/Art_Effect/Scenes/Particle/P_Escape_DropLightRed_01.P_Escape_DropLightRed_01", "地铁"},
        {"/Game/Arts/UI/TableIcons/WarzonerankTitle/CG028/CG027_Title_bg_QX.CG027_Title_bg_QX", "称号"},
        {"/Game/Arts_Commerce/2025Q1/IPSDS/Hard_Surface/Icon/Icon_Pendant_Bag_IPSDS_2.Icon_Pendant_Bag_IPSDS_2", "伪实体"},
        {"/Game/CSV/BattleItem_MODE_ForUpdate", "天线"},
        {"BP_STRUCT_LobbyAvatarExtEffect_type", "衣服动作"},
        {"BP_STRUCT_ItemEmotionRefer_type", "入场动作"}
    };
    // ===== 配置结束 =====

    extract_matching_files(UNPACK_DAT_PATH, MAIN_SCAN_DIR, STRING_TO_FOLDER);
    std::cout << coloredText("文件提取和处理完成！", 0, 255, 0) << std::endl;
}

// ===== 特征码提取功能实现 =====
struct PresetConfig {
    std::string name;
    int32_t index_value;
    std::vector<std::tuple<int32_t, int32_t>> feature_offsets;
    std::string subdir_name;
};

const std::vector<std::pair<int, PresetConfig>> PRESETS = {
    {1, {"淘汰播报特征码", 901142, {{-99, 2}, {-83, 2}}, "淘汰播报"}},
    {2, {"大厅动作特征码", 2201401, {{-186, 2}, {-170, 2}}, "大厅动作"}},
    {3, {"地铁特征码", 9807005, {{-41, 2}, {-25, 2}}, "地铁"}},
    {4, {"头像框特征码", 2002006, {{-182, 2}, {-166, 2}}, "头像框"}},
    {5, {"非块美化特征码", 413507, {{-33, 2}, {-17, 2}}, "美化"}},
    {6, {"伪实体特征码", 413507, {{-41, 2}, {-25, 2}}, "伪实体"}},
    {7, {"称号特征码", 3102404, {{-457, 2}, {-441, 2}}, "称号"}},
    {8, {"输出特征码", 413507, {{-17, 2}, {138, 2}}, "伪实体"}},
    {9, {"属性特征码", 413507, {{-41, 2}, {-25, 2}, {790, 2}}, "伪实体"}},
    {10, {"局内伪实体特征码", 413507, {{435, 2}, {485, 2}}, "美化"}}
};

std::string int_to_le_bytes(int32_t value) {
    std::string bytes;
    bytes.resize(4);
    for (int i = 0; i < 4; ++i) {
        bytes[i] = static_cast<char>(value & 0xFF);
        value >>= 8;
    }
    return bytes;
}

std::vector<std::string> find_features(const fs::path& file_path, 
                                     int32_t index_value, 
                                     const std::vector<std::tuple<int32_t, int32_t>>& feature_offsets) {
    std::vector<std::string> features;
    
    try {
        std::ifstream file(file_path, std::ios::binary);
        if (!file) return features;

        std::string content((std::istreambuf_iterator<char>(file)), 
                           std::istreambuf_iterator<char>());

        std::string hex_pattern = int_to_le_bytes(index_value);
        size_t pos = content.find(hex_pattern);
        if (pos == std::string::npos) {
            return features;
        }

        for (const auto& [offset, size] : feature_offsets) {
            size_t start = pos + offset;
            size_t end = start + size;
            
            if (start >= content.size() || end > content.size()) {
                features.emplace_back("");
                continue;
            }
            
            std::string feature_bytes(content.begin() + start, content.begin() + end);
            std::ostringstream hex_stream;
            hex_stream << std::hex << std::setfill('0');
            
            for (unsigned char c : feature_bytes) {
                hex_stream << std::setw(2) << static_cast<int>(c);
            }
            
            features.push_back(hex_stream.str());
        }
    } catch (...) {}
    
    return features;
}

bool save_features(const fs::path& output_file, const std::vector<std::string>& features) {
    try {
        std::ofstream file(output_file);
        if (!file) return false;
        
        for (const auto& feature : features) {
            file << feature << "\n";
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<fs::path> list_files(const fs::path& directory, const std::string& extension) {
    std::vector<fs::path> files;
    
    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == extension) {
                files.push_back(entry.path());
            }
        }
        std::sort(files.begin(), files.end());
    } catch (...) {}
    
    return files;
}

std::vector<std::string> run_preset(const PresetConfig& config) {
    fs::path selected_dir = MAIN_SCAN_DIR / config.subdir_name;
    if (!path_exists(selected_dir)) {
        return {};
    }
    
    auto dat_files = list_files(selected_dir, ".dat");
    if (dat_files.empty()) {
        return {};
    }
    
    fs::path file_path = dat_files[0];
    if (dat_files.size() > 1) {
        file_path = dat_files[0]; // 自动选择第一个文件
    }
    
    return find_features(file_path, config.index_value, config.feature_offsets);
}

void extract_all_features() {
    std::cout << gradientText("🔍 开始提取所有特征码", 0, 255, 255, 0, 0, 255) << std::endl;
    
    fs::create_directories(OUTPUT_DIR);
    std::unordered_map<std::string, std::vector<std::string>> results;
    
    for (const auto& [preset_id, config] : PRESETS) {
        if (config.name == "一键查找所有特征码" || config.name == "退出") continue;
        
        auto features = run_preset(config);
        if (!features.empty()) {
            results[config.name] = features;
            fs::path output_file = OUTPUT_DIR / (config.name + ".txt");
            save_features(output_file, features);
        }
    }
    
    if (!results.empty()) {
        std::cout << coloredText("所有特征码提取完成！", 0, 255, 0) << std::endl;
    } else {
        std::cout << coloredText("没有找到任何特征码", 255, 165, 0) << std::endl;
    }
}

// ===== 抓小包功能实现 =====
std::mutex cout_mutex;
std::mutex copy_mutex;
std::atomic<int> processed_files(0);
std::atomic<int> matched_files(0);

void generateVividColors(int& r, int& g, int& b) {
    const int colorSets[][3] = {{0, 255, 127}};
    int index = rand() % 1;
    r = colorSets[index][0];
    g = colorSets[index][1];
    b = colorSets[index][2];
}

std::unordered_map<char, size_t> build_bad_char_table(const std::vector<char>& pattern) {
    std::unordered_map<char, size_t> bad_char;
    size_t len = pattern.size();
    
    for (size_t i = 0; i < len - 1; ++i) {
        bad_char[pattern[i]] = len - i - 1;
    }
    
    return bad_char;
}

bool boyer_moore_search(const std::vector<char>& text, const std::vector<char>& pattern) {
    if (pattern.empty()) return true;
    if (text.size() < pattern.size()) return false;
    
    auto bad_char = build_bad_char_table(pattern);
    size_t pat_len = pattern.size();
    size_t txt_len = text.size();
    size_t shift = 0;
    
    while (shift <= (txt_len - pat_len)) {
        int j = pat_len - 1;
        
        while (j >= 0 && pattern[j] == text[shift + j]) {
            j--;
        }
        
        if (j < 0) {
            return true;
        } else {
            char bad_char_val = text[shift + j];
            size_t bad_char_shift = bad_char.find(bad_char_val) != bad_char.end() 
                                   ? bad_char[bad_char_val] 
                                   : pat_len;
            shift += std::max(1, static_cast<int>(bad_char_shift) - (static_cast<int>(pat_len) - 1 - j));
        }
    }
    
    return false;
}

std::unique_ptr<std::vector<char>> read_file(const fs::path& filepath) {
    try {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file) return nullptr;

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        auto buffer = std::make_unique<std::vector<char>>(size);
        if (!file.read(buffer->data(), size)) {
            return nullptr;
        }

        return buffer;
    } catch (...) {
        return nullptr;
    }
}

void process_file(const fs::path& filepath, 
                 const std::vector<char>& target_content,
                 const fs::path& output_dir) {
    try {
        auto current_content = read_file(filepath);
        if (!current_content) return;
        
        if (current_content->size() > target_content.size()) {
            processed_files++;
            return;
        }
        
        bool found = boyer_moore_search(target_content, *current_content);
        
        if (found) {
            std::lock_guard<std::mutex> lock(copy_mutex);
            fs::path output_path = output_dir / filepath.filename();
            fs::copy_file(filepath, output_path, fs::copy_options::overwrite_existing);
            
            int r, g, b;
            generateVividColors(r, g, b);
            std::lock_guard<std::mutex> lock_cout(cout_mutex);
            std::cout << coloredText("找到匹配文件: ", r, g, b) << filepath.string() << std::endl;
            
            matched_files++;
        }
        
        processed_files++;
    } catch (...) {}
}

void find_binary_parts(const std::vector<fs::path>& target_files, 
                      const fs::path& search_dir, 
                      const fs::path& output_dir) {
    fs::create_directories(output_dir);
    std::vector<fs::path> file_paths;
    
    for (const auto& entry : fs::recursive_directory_iterator(search_dir)) {
        if (entry.is_regular_file()) {
            file_paths.push_back(entry.path());
        }
    }
    
    int num_threads = std::thread::hardware_concurrency();
    
    for (const auto& target_file : target_files) {
        auto target_content = read_file(target_file);
        if (!target_content) continue;

        processed_files = 0;
        matched_files = 0;
        
        std::vector<std::thread> threads;
        size_t files_per_thread = file_paths.size() / num_threads;
        
        auto worker = [&](size_t start, size_t end) {
            for (size_t i = start; i < end && i < file_paths.size(); ++i) {
                process_file(file_paths[i], *target_content, output_dir);
            }
        };
        
        for (int i = 0; i < num_threads; ++i) {
            size_t start = i * files_per_thread;
            size_t end = (i == num_threads - 1) ? file_paths.size() : (i + 1) * files_per_thread;
            threads.emplace_back(worker, start, end);
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
}

void clear_small_pack_directory(const fs::path& dir) {
    try {
        for (const auto& entry : fs::directory_iterator(dir)) {
            fs::remove_all(entry.path());
        }
    } catch (...) {}
}

void copy_files_back(const fs::path& src_dir, const fs::path& dst_dir) {
    try {
        if (!path_exists(src_dir)) return;
        fs::create_directories(dst_dir);
        
        for (const auto& entry : fs::directory_iterator(src_dir)) {
            if (entry.is_regular_file()) {
                fs::path dest_path = dst_dir / entry.path().filename();
                fs::copy_file(entry.path(), dest_path, fs::copy_options::overwrite_existing);
            }
        }
    } catch (...) {}
}

void grab_small_packs() {
    std::cout << gradientText("🎒 开始抓取小包", 255, 215, 0, 255, 105, 180) << std::endl;
    
    srand(time(0));
    clear_small_pack_directory(SMALL_PACK_DIR);

    std::vector<fs::path> target_dirs = {
        MAIN_SCAN_DIR / "头像框",
        MAIN_SCAN_DIR / "淘汰播报",
        MAIN_SCAN_DIR / "地铁",
        MAIN_SCAN_DIR / "大厅动作",
        MAIN_SCAN_DIR / "称号",
        MAIN_SCAN_DIR / "伪实体",
        MAIN_SCAN_DIR / "美化"
    };
    
    std::vector<fs::path> target_files;
    for (const auto& dir : target_dirs) {
        if (path_exists(dir)) {
            for (const auto& entry : fs::directory_iterator(dir)) {
                if (entry.is_regular_file()) {
                    target_files.push_back(entry.path());
                }
            }
        }
    }
    
    if (target_files.empty()) {
        std::cout << coloredText("没有找到任何目标文件!", 255, 0, 0) << std::endl;
        return;
    }

    auto start_time = std::chrono::steady_clock::now();
    find_binary_parts(target_files, UNPACK_UEXP_PATH, SMALL_PACK_DIR);
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    
    std::cout << coloredText("搜索完成! 耗时: ", 0, 191, 255) 
              << gradientText(std::to_string(duration.count()) + " 秒", 255, 215, 0, 255, 105, 180) << std::endl;

    clear_small_pack_directory(UNPACK_UEXP_PATH);
    copy_files_back(SMALL_PACK_DIR, UNPACK_UEXP_PATH);
    std::cout << coloredText("小包抓取完成！", 0, 255, 0) << std::endl;
}

// ===== 主流程控制 =====
void run_full_automation() {
    std::cout << gradientText("🚀 开始全自动处理流程", 255, 0, 0, 255, 255, 0) << std::endl;
    
    // 首先让用户选择pak文件
    std::cout << gradientText("=== 选择pak文件 ===", 0, 255, 255, 0, 0, 255) << std::endl;
    if (!select_pak_file_from_directory(selected_pak_file)) {
        return;
    }
    
    // 1. 块解包
    std::cout << gradientText("=== 步骤1: 块解包 ===", 255, 0, 0, 255, 255, 0) << std::endl;
    unpack_dat();
    
    // 2. 非块解包
    std::cout << gradientText("=== 步骤2: 非块解包 ===", 0, 255, 255, 0, 0, 255) << std::endl;
    unpack_uexp();
    
    // 3. 提取dat
    std::cout << gradientText("=== 步骤3: 提取dat文件 ===", 255, 215, 0, 255, 105, 180) << std::endl;
    extract_dat_files();
    
    // 4. 提取所有特征码
    std::cout << gradientText("=== 步骤4: 提取所有特征码 ===", 0, 191, 255, 138, 43, 226) << std::endl;
    extract_all_features();
    
    // 5. 抓小包
    std::cout << gradientText("=== 步骤5: 抓取小包 ===", 255, 105, 180, 255, 215, 0) << std::endl;
    grab_small_packs();
    
    std::cout << gradientText("🎉 存值流程完成！", 0, 255, 0, 0, 191, 255) << std::endl;
}

int main() {

    

    // 创建必要的目录
    fs::create_directories(UNPACK_DAT_PATH);
    fs::create_directories(UNPACK_UEXP_PATH);
    fs::create_directories(MAIN_SCAN_DIR);
    fs::create_directories(OUTPUT_DIR);
    fs::create_directories(SMALL_PACK_DIR);
    
    // 运行全自动流程
    run_full_automation();
    
    return 0;
}