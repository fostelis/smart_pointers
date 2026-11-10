#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>
namespace fs = std::filesystem;
struct Data {
    std::vector<std::string> columns;
    std::vector<std::vector<double>> rows;
};
Data read_csv(const fs::path& path) {
    Data data;
    std::ifstream file(path);
    std::string line;
    if (!std::getline(file, line)) {
        return data;
    }
    std::stringstream header(line);
    std::string value;
    while (std::getline(header, value, ',')) {
        if (!value.empty() && value.back() == '\r') {
            value.pop_back();
        }
        data.columns.push_back(value);
    }
    while (std::getline(file, line)) {
        std::stringstream input(line);
        std::vector<double> row;
        bool valid = true;
        while (std::getline(input, value, ',')) {
            try {
                row.push_back(std::stod(value));
            } catch (...) {
                valid = false;
            }
        }
        if (valid && row.size() == data.columns.size()) {
            data.rows.push_back(row);
        }
    }
    return data;
}
std::string escape_html(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (c == '&') { result += "&amp;"; }
        else if (c == '<') { result += "&lt;"; }
        else if (c == '>') { result += "&gt;"; }
        else if (c == '"') { result += "&quot;"; }
        else { result += c; }
    }
    return result;
}
std::string read_text(const fs::path& path) {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
std::string attribute(const std::string& tag, const std::string& key) {
    const std::string search = key + "=\"";
    size_t pos = tag.find(search);
    if (pos == std::string::npos) {
        return "";
    }
    pos += search.size();
    size_t end = tag.find('"', pos);
    return end == std::string::npos ? "" : tag.substr(pos, end - pos);
}
std::string chart_svg(const Data& data) {
    if (data.rows.empty() || data.columns.size() < 2) {
        return "<p>No data</p>";
    }
    const std::vector<std::string> colors = {"#60a5fa", "#34d399", "#f59e0b", "#f472b6", "#a78bfa", "#22d3ee"};
    double max_value = 0;
    for (const auto& row : data.rows) {
        for (size_t j = 1; j < row.size(); j++) {
            max_value = std::max(max_value, row[j]);
        }
    }
    if (max_value <= 0) {
        max_value = 1;
    }
    std::ostringstream out;
    out << "<svg viewBox='0 0 900 350' role='img' aria-label='Performance chart' style='width:100%;height:auto'>";
    for (int i = 0; i <= 4; i++) {
        double y = 280.0 - i * 60.0;
        out << "<line x1='75' y1='" << y << "' x2='860' y2='" << y << "' stroke='#334155'/>";
        out << "<text x='65' y='" << y + 4 << "' text-anchor='end' fill='#94a3b8' font-size='12'>" << std::fixed << std::setprecision(1) << max_value * i / 4 << "</text>";
    }
    for (size_t i = 0; i < data.rows.size(); i++) {
        double x = 95.0 + (data.rows.size() == 1 ? 0 : 740.0 * i / (data.rows.size() - 1));
        out << "<text x='" << x << "' y='305' text-anchor='middle' fill='#cbd5e1' font-size='12'>" << static_cast<long long>(data.rows[i][0]) << "</text>";
    }
    for (size_t j = 1; j < data.columns.size(); j++) {
        std::string color = colors[(j - 1) % colors.size()];
        out << "<polyline fill='none' stroke='" << color << "' stroke-width='3' points='";
        for (size_t i = 0; i < data.rows.size(); i++) {
            double x = 95.0 + (data.rows.size() == 1 ? 0 : 740.0 * i / (data.rows.size() - 1));
            double y = 280.0 - 240.0 * data.rows[i][j] / max_value;
            out << x << "," << y << " ";
        }
        out << "'/>";
        for (size_t i = 0; i < data.rows.size(); i++) {
            double x = 95.0 + (data.rows.size() == 1 ? 0 : 740.0 * i / (data.rows.size() - 1));
            double y = 280.0 - 240.0 * data.rows[i][j] / max_value;
            out << "<circle cx='" << x << "' cy='" << y << "' r='4' fill='" << color << "'><title>" << escape_html(data.columns[j]) << ": " << data.rows[i][j] << "</title></circle>";
        }
    }
    out << "</svg><div class='legend'>";
    for (size_t j = 1; j < data.columns.size(); j++) {
        out << "<span><b style='background:" << colors[(j - 1) % colors.size()] << "'></b>" << escape_html(data.columns[j]) << "</span>";
    }
    out << "</div>";
    return out.str();
}
void add_section(std::ofstream& out, const fs::path& csv, const std::string& title, const std::string& unit) {
    Data data = read_csv(csv);
    out << "<section><h2>" << title << "</h2><p class='muted'>" << unit << "</p>";
    if (data.rows.empty()) {
        out << "<p>CSV not found or empty: " << escape_html(csv.string()) << "</p></section>";
        return;
    }
    out << "<div class='scroll'><table><thead><tr>";
    for (const auto& name : data.columns) {
        out << "<th>" << escape_html(name) << "</th>";
    }
    out << "</tr></thead><tbody>";
    for (const auto& row : data.rows) {
        out << "<tr>";
        for (size_t i = 0; i < row.size(); i++) {
            out << "<td>" << std::fixed << std::setprecision(i == 0 ? 0 : 3) << row[i] << "</td>";
        }
        out << "</tr>";
    }
    out << "</tbody></table></div><h3>График</h3>" << chart_svg(data) << "</section>";
}

int run(const fs::path& executable, const std::string& args, const fs::path& log) {
    fs::path old_directory = fs::current_path();
    fs::current_path(log.parent_path().parent_path());
    std::string command = "cmd /C \"\"" + executable.string() + "\" " + args
        + " > \"" + log.string() + "\" 2>&1\"";
    int result = std::system(command.c_str());
    fs::current_path(old_directory);
    return result;
}


int main(int argc, char* argv[]) {
    const fs::path directory = fs::absolute(fs::path(argv[0])).parent_path();
    const fs::path project = argc > 1 ? fs::absolute(argv[1]) : fs::current_path();
    const fs::path results = project / "results";
    fs::create_directories(results);
    const int test_code = run(directory / "smart_pointers_tests.exe", "--gtest_color=no --gtest_output=xml:\"" + (results / "tests.xml").string() + "\"", results / "tests.log");
    std::cout << "Tests finished, code: " << test_code << "\n";
    int bench_code = run(directory / "smart_pointers_benchmark.exe", "", results / "benchmark.log");
    std::cout << "Time benchmarks finished, code: " << bench_code << "\n";
    int memory_code = run(directory / "smart_pointers_memory.exe", "", results / "memory.log");
    std::cout << "Memory benchmark finished, code: " << memory_code << "\n";
    for (const std::string& name : {"benchmark_results.csv", "copy_results.csv", "memory_results.csv"}) {
        fs::path generated = project / name;
        fs::path destination = results / name;
        if (fs::exists(generated)) {
            fs::remove(destination);
            fs::copy_file(generated, destination);
        }
    }
    std::string xml = read_text(results / "tests.xml");
    std::string test_tag;
    size_t start = xml.find("<testsuites ");
    if (start != std::string::npos) {
        size_t end = xml.find('>', start);
        test_tag = xml.substr(start, end - start);
    }
    std::string tests = attribute(test_tag, "tests");
    std::string failures = attribute(test_tag, "failures");
    std::string errors = attribute(test_tag, "errors");
    bool passed = test_code == 0 && !tests.empty() && failures == "0" && errors == "0";
    std::ofstream out(results / "dashboard.html");
    out << R"(<!doctype html><html lang='ru'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Smart Pointers Dashboard</title><style>*{box-sizing:border-box}body{margin:0;background:#080808;color:#fff;font:15px Arial,sans-serif;line-height:1.5}.wrap{max-width:1100px;margin:0 auto;padding:40px 24px}h1{margin:0 0 10px;color:#d74b4b;font-size:30px}h2{margin:0 0 10px;color:#d74b4b;font-size:21px}h3{margin-top:28px;color:#d74b4b;font-size:17px}.muted{color:#aaa}section{margin-top:24px;padding:24px;background:#202020;border:1px solid #555;border-radius:0;box-shadow:none}.status{display:inline-block;padding:8px 12px;background:#303030;color:#fff;border:1px solid #666;border-radius:0;font-weight:600}.bad{background:#351818;color:#ff7777;border-color:#a33b3b}.scroll{overflow-x:auto}table{width:100%;margin-top:18px;border-collapse:collapse}th,td{padding:11px 12px;text-align:right;border:1px solid #444;color:#fff}th:first-child,td:first-child{text-align:left}th{background:#303030;font-weight:600}tr:nth-child(even){background:#262626}.legend{display:flex;flex-wrap:wrap;gap:16px;margin:16px 0}.legend span{display:flex;align-items:center;gap:7px;font-size:13px;color:#fff}.legend b{display:inline-block;width:12px;height:12px;border-radius:0}a{color:#d74b4b}pre{padding:16px;background:#111;border:1px solid #444;border-radius:0;color:#fff;white-space:pre-wrap;overflow-wrap:anywhere;font-size:12px}summary{cursor:pointer;color:#fff}svg{background:#202020}</style></head><body><main class='wrap'><h1>Smart Pointers — Dashboard</h1><p class='muted'>Тесты GoogleTest и измерения производительности · Release</p>)";
    out << "<section><h2>Результаты тестов</h2><p class='status" << (passed ? "" : " bad") << "'>";
    out << (passed ? "Все тесты пройдены: " + tests + " / " + tests : "Проверка не пройдена или XML недоступен") << "</p>";
    out << "<p>Код завершения тестов: " << test_code << ". Временные эксперименты: " << bench_code << ". Память: " << memory_code << ".</p>";
    out << "<details><summary>Подробный лог GoogleTest</summary><pre>" << escape_html(read_text(results / "tests.log")) << "</pre></details></section>";
    add_section(out, results / "benchmark_results.csv", "Создание и уничтожение", "Время в миллисекундах, медиана пяти запусков");
    add_section(out, results / "copy_results.csv", "Копирование и перемещение", "Время в миллисекундах, медиана пяти запусков");
    add_section(out, results / "memory_results.csv", "Потребление памяти", "Изменение Working Set, МБ. Приближённая оценка, чувствительная к порядку экспериментов");
    out << "<p class='muted'>Файлы результатов сохранены в папке results. Графики встроены в HTML и работают без интернета.</p></main></body></html>";
    out.close();
    fs::path report = fs::absolute(results / "dashboard.html");
    std::cout << "Report: " << report << "\n";
    ShellExecuteW(nullptr, L"open", report.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    return passed && bench_code == 0 && memory_code == 0 ? 0 : 1;
}
