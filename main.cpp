//
// Created by Jazmin Selene Ortega on 24/03/26.
//

#include <iostream>
#include "SplayTree.h"

int main() {
    SplayTree BookRecommender;
    BookRecommender.insert(1234, "Pride and Prejudice", 10);
    BookRecommender.insert(6789, "The Picture of Dorian Gray", 12);
    return 0;
}