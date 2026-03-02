#include <sstream>
#include <QApplication>
#include <QPushButton>
#include <yaml-cpp/yaml.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    const YAML::Node root = YAML::Load("answer: 42\ntext: Hello");
    std::stringstream ss;
    ss << root["text"].as<std::string>() << " ";
    ss << root["answer"].as<int>();
    QPushButton button(ss.str().c_str(), nullptr);
    button.resize(200, 100);
    button.show();
    return QApplication::exec();
}