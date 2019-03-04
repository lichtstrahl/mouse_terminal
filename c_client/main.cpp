#include <iostream>
#include <unistd.h>
#include "timer.h"
#include "FileHolder.h"
#include <fstream>
#include "const.h"
#include <vector>
#include <string>
#include <string.h>

using std::ifstream;
using std::vector;
using std::string;

#define MAX_COUNT "10"
#define DATA_PATH "/proc/mouseListener/info"
#define VERSION "v.2.1.0 Добавлены специальные символы [Space] [Backspace]"


#define MSG_MAX_CONT_ACTION "Всего будет обработано " MAX_COUNT " действий"

class Model
{
private:
    int currentIndex;
    int currentSet;
    vector<string> buffer;

    string getCurrentElement() {
        return availableSet[currentSet].set[currentIndex];
    }
    Set getCurrentSet() {
        return availableSet[currentSet];
    }

public:
    Model()
    {
        this->currentIndex = 0;
        this->currentSet = 0;
    }

    void print() {
        printf("Текущее множество: %s;\n", availableSet[currentSet].name.c_str());
        printf("Текущий элемент: %s;\n", getCurrentElement().c_str());
        printf("Буффер: %s;\n", buffer.size() != 0 ? buffer_to_str() : MSG_NEXT_CLICK_EXIT);
    }

    char* buffer_to_str() {
        static char buf[1024];
        int i;
        for (i = 0; i < buffer.size(); i++)
            buf[i] = buffer[i].c_str()[0];
        buf[i] = '\0';
        return buf;
    }

    bool processingAction(Action a) {
        switch (a.type) {
            case CLICK_LEFT:
                processingClickLeft();
                break;
            case CLICK_RIGHT:
                nextSet();
                break;

            case CLICK_MIDDLE: {
                unsigned long n = buffer.size();
                system(buffer_to_str());
                buffer.clear();
                return n == 0;
            }


            case SCROLL_UP:
                forwardElement();
                break;

            case SCROLL_DOWN:
                backElement();
                break;
            default:
                break;
        }

        return false;

    }

    void processingClickLeft() {

        switch (getCurrentSet().type) {
            case SetType::COMMON:
                buffer.push_back(getCurrentElement());
                break;

            case SetType::SPECIFIC: {
                string action = getCurrentElement();
                if (!strcmp(action.c_str(), SPECIAL_SPACE) && buffer.size() > 0) {   // Обработка пробела
                    buffer.push_back(" ");
                }

                if (!strcmp(action.c_str(), SPECIAL_BACKSPACE) && buffer.size() > 0) {
                    buffer.pop_back();
                }


                break;
            }

        }
    }

    void nextSet() {
        if (++currentSet == COUNT_AVAILABLE_SET) {
            currentSet = 0;
        }
        currentIndex = 0;
    }

    void forwardElement() {
        if (++currentIndex == availableSet[currentSet].length) currentIndex = 0;
    }

    void backElement() {
        if (--currentIndex < 0) currentIndex = availableSet[currentSet].length-1;
    }

};

int main(int argc, char **argv) {

    FileHolder fileHolder(DATA_PATH);
    Action oldAction = {-1, CLICK_LEFT, nullptr};
    Model model;
    vector<Action> bufferAction;

    while (true) {
        Action newAction = fileHolder.readFile();
        if (newAction.id != oldAction.id) {
            system("clear");
            SEND_MSG(VERSION);
            bool exit = model.processingAction(newAction);
            bufferAction.push_back(newAction);
            model.print();
            oldAction = newAction;

            if (exit) {
                break;
            }
        }

        PAUSE_MS(200);
    }


    system("bash ./stop.sh");
    return 0;
}