package main

import (
	"bytes"
	"context"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"strings"
	"sync"
	"syscall"
	"time"
)

var counter = struct {
	sync.RWMutex
	visited map[string]int
}{visited: make(map[string]int)}

var expireTime = struct {
	year  int
	month time.Month
	day   int
}{}

var logWriter = struct {
	logger *log.Logger
	fp     *os.File
}{}

type Feedback struct {
	Title       string `json:"title"`
	Category    string `json:"category"`
	Release     string `json:"release"`
	Codename    string `json:"codename"`
	Description string `json:"description"`
}

type Issue struct {
	Summary     string `json:"summary"`
	Description string `json:"description"`
	Category    string `json:"category"`
	Project     string `json:"project"`
}

func setCounterNow() {
	counter.Lock()
	defer counter.Unlock()
	counter.visited = make(map[string]int)
}

func validateFeedback(r *http.Request) bool {
	checkExpireTime()

	counter.RLock()
	n := counter.visited[r.RemoteAddr]
	counter.RUnlock()

	if n >= 10 {
		return false
	}

	counter.Lock()
	defer counter.Unlock()
	counter.visited[r.RemoteAddr]++

	return true
}

func checkExpireTime() {
	year, month, day := time.Now().Date()

	if expireTime.year != year ||
		expireTime.month != month ||
		expireTime.day != day {
		gfbInit()
	}
}

func makeRequest(fb *Feedback) (*http.Request, error) {
	//const TOKEN string = "lREDdOh5jvzQcRclffDG7njhCLfoMGJw" // Administrator
    const TOKEN string = "u-cSfjFwHnCTeVcIZclp2nuQS3fwL8-7" // Reporter
	const BTS string = "http://feedback.gooroom.kr/mantis/api/rest/issues/"

	issue := fmt.Sprintf(`{"summary": "%s", "description": "%s", "category": {"name": "%s"}, "project": {"name": "%s"}}`,
		fb.Title, fb.Description, fb.Category, "Gooroom Feedback")
	req, err := http.NewRequest("POST", BTS, bytes.NewBuffer(bytes.NewBufferString(issue).Bytes()))
	if err != nil {
		return nil, err
	}

	defer req.Body.Close()
	req.Header.Add("Authorization", TOKEN)
	req.Header.Add("Content-Type", "application/json")

	return req, nil
}

func getFeedback(r *http.Request) (*Feedback, error) {
	var title, category, release, codename, description string

	if err := r.ParseForm(); err != nil {
		return nil, err
	}

	for k, v := range r.Form {
		switch strings.ToLower(k) {
		case "title":
			title = v[0]
		case "category":
			category = v[0]
		case "release":
			release = v[0]
		case "codename":
			codename = v[0]
		case "description":
			description = v[0]
		}
	}

	return &Feedback{Title: title, Category: category, Release: release, Codename: codename, Description: description}, nil
}

func statusHandler(w http.ResponseWriter, r *http.Request) {
	logWriter.logger.Printf("[Gooroom-Feedback-Server] => Request Received. (Status, Sender: %s)\n", r.RemoteAddr)
    fmt.Printf("[Gooroom-Feedback-Server] => Request Received. (Status, Sender: %s)\n", r.RemoteAddr) // DELETE

	if r.Method == "GET" {
		w.WriteHeader(http.StatusOK)
		w.Write([]byte(http.StatusText(http.StatusOK)))

		logWriter.logger.Println("[Gooroom-Feedback-Server] => Running...")
		fmt.Println("[Gooroom-Feedback-Server] => Running...") // DELETE
	} else {
		w.WriteHeader(http.StatusMethodNotAllowed)
		w.Write([]byte(http.StatusText(http.StatusMethodNotAllowed)))

		logWriter.logger.Println("[Gooroom-Feedback-Server] => Not Allowed Method.")
		fmt.Println("[Gooroom-Feedback-Server] => Not Allowed Method.") // DELETE
	}
}

func feedbackHandler(w http.ResponseWriter, r *http.Request) {
	logWriter.logger.Printf("[Gooroom-Feedback-Server] => Request Received. (Feedback, Sender: %s)\n", r.RemoteAddr)
	fmt.Printf("[Gooroom-Feedback-Server] => Request Received. (Feedback, Sender: %s)\n", r.RemoteAddr) // DELETE

	if r.Method == "POST" {
		if validateFeedback(r) {
			logWriter.logger.Println("[Gooroom-Feedback-Server] => Request Validated.")
			logWriter.logger.Println("=========================== [FEEDBACK] ===========================")
			logWriter.logger.Printf("Method: %s, URL: %s, Proto: %s\n", r.Method, r.URL, r.Proto)
			fmt.Println("[Gooroom-Feedback-Server] => Request Validated.") // DELETE
			fmt.Println("=========================== [FEEDBACK] ===========================") // DELETE
			fmt.Printf("Method: %s, URL: %s, Proto: %s\n", r.Method, r.URL, r.Proto) // DELETE
			for k, v := range r.Header {
				logWriter.logger.Printf("%q = %q\n", k, v)
			}

			fb, err := getFeedback(r)
			if err != nil {
				logWriter.logger.Printf("[Gooroom-Feedback-Server] => %s\n", err)
				fmt.Printf("[Gooroom-Feedback-Server] => %s\n", err) // DELETE

				w.WriteHeader(http.StatusBadRequest)
				w.Write([]byte(http.StatusText(http.StatusBadRequest)))

				return // TODO
			}

			logWriter.logger.Printf("%+v\n", fb)
			logWriter.logger.Println("==================================================================")
			fmt.Printf("%+v\n", fb) // DELETE
			fmt.Println("==================================================================") // DELETE

			req, err := makeRequest(fb)
			if err != nil {
				logWriter.logger.Printf("[Gooroom-Feedback-Server] => %s\n", err)
				fmt.Printf("[Gooroom-Feedback-Server] => %s\n", err) // DELETE

				w.WriteHeader(http.StatusInternalServerError)
				w.Write([]byte(http.StatusText(http.StatusInternalServerError)))

				return // TODO
			}

			logWriter.logger.Println("[Gooroom-Feedback-Server] => Issue Created")
			fmt.Println("[Gooroom-Feedback-Server] => Issue Created") // DELETE
			fmt.Printf("%+v\n", req) // DELETE

			client := &http.Client{}
			resp, err := client.Do(req)
			if err != nil {
				w.WriteHeader(http.StatusInternalServerError)
				w.Write([]byte(http.StatusText(http.StatusInternalServerError)))

				logWriter.logger.Println("[Gooroom-Feedback-Server] => Internal Server Error.")
				logWriter.logger.Printf("ERROR: %s\n", err)
				fmt.Println("[Gooroom-Feedback-Server] => Internal Server Error.") // DELETE
				fmt.Printf("ERROR: %s\n", err) // DELETE
			}
			defer resp.Body.Close()

			logWriter.logger.Println("[Gooroom-Feedback-Server] => Issue Requested.")
			fmt.Println("[Gooroom-Feedback-Server] => Issue Requested.") // DELETE

            logWriter.logger.Println(resp)
            fmt.Println(resp.StatusCode) // DELETE
            fmt.Println(resp) // DELETE

            w.WriteHeader(resp.StatusCode)
            w.Write([]byte(http.StatusText(resp.StatusCode)))
		} else {
			w.WriteHeader(http.StatusNotAcceptable)
			w.Write([]byte(http.StatusText(http.StatusNotAcceptable)))

			logWriter.logger.Println("[Gooroom-Feedback-Server] => Request Invalidated.")
			fmt.Println("[Gooroom-Feedback-Server] => Request Invalidated.") // DELETE
		}
	} else {
		w.WriteHeader(http.StatusMethodNotAllowed)
		w.Write([]byte(http.StatusText(http.StatusMethodNotAllowed)))

		logWriter.logger.Println("[Gooroom-Feedback-Server] => Not Allowed Method.")
		fmt.Println("[Gooroom-Feedback-Server] => Not Allowed Method.") // DELETE
	}
}

func setExpireTimeNow() {
	expireTime.year, expireTime.month, expireTime.day = time.Now().Date()
}

func setLogWriterNow() {
	if logWriter.fp != nil {
		if err := logWriter.fp.Close(); err != nil {
			panic(err)
		}
	}

	logFile := fmt.Sprintf("gfb-%d-%02d-%02d.log", expireTime.year, expireTime.month, expireTime.day)

	fp, err := os.OpenFile(logFile, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0644)
	if err != nil {
		panic(err)
	}

	logWriter.fp = fp
	logWriter.logger = log.New(logWriter.fp, "", log.LstdFlags|log.Lshortfile)
	logWriter.logger.Printf("[Gooroom-Feedback-Server] => Date: %d-%02d-%02d\n",
		expireTime.year, expireTime.month, expireTime.day)
	fmt.Printf("[Gooroom-Feedback-Server] => Date: %d-%02d-%02d\n", // DELETE
		expireTime.year, expireTime.month, expireTime.day) // DELETE
}

func gfbInit() {
	setExpireTimeNow()

	setCounterNow()

	setLogWriterNow()

	logWriter.logger.Println("[Gooroom-Feedback-Server] => Initialized.")
	fmt.Println("[Gooroom-Feedback-Server] => Initialized.") //DELETE
}

func main() {
	gfbInit()

	srv := &http.Server{
		Addr:         ":8000",
		WriteTimeout: 2 * time.Second,
		ReadTimeout:  1 * time.Second,
		Handler:      nil,
	}

	http.HandleFunc("/gooroom/feedback/new", feedbackHandler)
	http.HandleFunc("/status", statusHandler)

	idleConnsClosed := make(chan struct{})

	go func() {
		done := make(chan os.Signal, 1)
		signal.Notify(done, os.Interrupt, syscall.SIGINT, syscall.SIGTERM)

		<-done

		if err := srv.Shutdown(context.Background()); err != nil {
			logWriter.logger.Printf("[Gooroom-Feedback-Server] => %v", err)
			fmt.Printf("[Gooroom-Feedback-Server] => %v", err) // DELETE
		}

		close(idleConnsClosed)
	}()

	logWriter.logger.Println("[Gooroom-Feedback-Server] => Serving...")
	fmt.Println("[Gooroom-Feedback-Server] => Serving...") // DELETE

	if err := srv.ListenAndServe(); err != nil {
		logWriter.logger.Fatalf("[Gooroom-Feedback-Server] => %v", err)
	}

	<-idleConnsClosed
}
