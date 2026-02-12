## Any

В этом задании требуется реализовать аналог класса [std::any](https://en.cppreference.com/w/cpp/utility/any).

Данный класс позволяет хранить в себе значение любого типа:
```
Any a = 5;
Any b = std::vector<int>{5, 5};
```

Примеры использования смотрите в тестирующем коде, файл main.cpp.

### Примечания

* Использование `std::any` и `void *` запрещено.
* В решении вам понадобится техника type erasure, прочитать о которой можно [здесь](https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Type_Erasure).
