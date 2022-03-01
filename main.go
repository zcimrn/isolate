package main

/*
#include "kek.h"
*/
import "C"

import "fmt"

func main() {
    fmt.Println("calling from go")
    C.run()
    fmt.Println("exiting from go")
}
