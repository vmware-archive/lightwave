package main

import (
	"fmt"
	"io"
	"sync"
	"net"
	"io/ioutil"
	"net/http"
	"os"
	"time"
)


	var lock sync.Mutex
	var totalTimeTaken float64
	var longestTimeTaken float64
	var droppedPackets uint
	var totalPackets uint
	var errorPackets uint
  const runningTimes = 100

func main() {
	start := time.Now()
	spinnerStopChan := make(chan struct{})
	go spinner(100 * time.Millisecond, spinnerStopChan)
	totalTimeTaken = 0
	longestTimeTaken = 0
	droppedPackets = 0
	totalPackets = 0
	errorPackets = 0
	var wg sync.WaitGroup
	fetch (os.Args[1])
	for i := 0; i  < runningTimes; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			fetch (os.Args[1])
		}()
	}
	wg.Wait()
	close(spinnerStopChan)
	fmt.Printf("%.2fs elapsed\n", time.Since(start).Seconds())
	fmt.Printf("Time Per Request: %.2f\n", totalTimeTaken/float64(totalPackets))
	fmt.Printf("Longest Delay: %.2f\n", longestTimeTaken)
	fmt.Printf("Dropped Packets: %d\n", droppedPackets)
	fmt.Printf("Errors : %d\n", errorPackets)
}

func spinner(delay time.Duration, spinnerStopChan chan struct{}) {
	for {
		select {
		default:
			for _, r := range `-\|/` {
				fmt.Printf("\r%c", r)
				time.Sleep(delay)
			}
		case <- spinnerStopChan:
			return
		}
	}
}

func fetch(url string) {
	start := time.Now()
	resp, err := http.Get(url)
	packetDropped := false
	errorInPacket := false
	if err != nil {
		fmt.Fprintf (os.Stderr, "Error: %v\n", err)
		if neterror, ok := err.(net.Error); ok && neterror.Timeout() {
        fmt.Fprintf(os.Stderr, "Timeout Error\n")
        packetDropped = true
      	}
	} else {
		_, err := io.Copy(ioutil.Discard, resp.Body)
		resp.Body.Close()
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error %s: %v", url, err)
			errorInPacket = true
		}
		if (resp.StatusCode != 200) {
			fmt.Fprintf(os.Stderr, "Error %s: %d", url, resp.StatusCode)
			errorInPacket = true
		}
	}
	secs := time.Since(start).Seconds()

	lock.Lock()
	totalPackets++
	totalTimeTaken += secs
	if secs > longestTimeTaken {
		longestTimeTaken = secs
	}
	if packetDropped {
		droppedPackets++
	}

	if errorInPacket {
		errorPackets++
	}
	lock.Unlock()
}
