package main

// #cgo LDFLAGS: -lcap
// #include "isolate.h"
import "C"

import "fmt"

func main() {
    fmt.Println("[ GO ] entering main func")
    //C.init();
    C.run();
    //C.clean();
    fmt.Println("[ GO ] exiting main func")
}
