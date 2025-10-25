#include "browser/storageBrowser.h"

int main() {
    // Создаем хранилище и добавляем тестовые данные
    auto storage = std::make_shared<data::storage>();
    
    // Добавляем папки и сниппеты
    auto folder1_uuid = storage->addFolder("Projects");
    auto folder2_uuid = storage->addFolder("Work");
    storage->addSnippet("Hello World", "print('Hello World')");
    
    storage->folderDown(folder1_uuid);
    storage->addSnippet("C++ Code", "#include <iostream>");
    
    // Запускаем браузер
    ui::runStoragwBrowser(storage);

    return 0;
}