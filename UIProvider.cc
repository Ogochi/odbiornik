#include "UIProvider.h"


void UIProvider::setUpSocket() {

}

void UIProvider::run() {
    setUpSocket();
}

UIProvider::~UIProvider() {
    close(sock);
}