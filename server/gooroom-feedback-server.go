package main

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	//"io/ioutil" // DELETE
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
	sync.RWMutex
	year  int
	month time.Month
	day   int
    min int
}{}

var logWriter = struct {
    sync.RWMutex
    logger *log.Logger
    fp *os.File
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

func resetCounter() {
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
	expireTime.Lock()
	defer expireTime.Unlock()
	year, month, day := time.Now().Date()
	if expireTime.year != year ||
		expireTime.month != month ||
        expireTime.min != time.Now().Minute() || // DELETE
		expireTime.day != day {
        gfbInit()
	}
}

/* DELETE
func issueHandler(w http.ResponseWriter, r *http.Request) {
		logger.Printf("RemoteAddr: %s\n", r.RemoteAddr)
		if r.Method == "POST" {
		    logger.Println("===ISSUE===")
			logger.Printf("%s %s %s\n", r.Method, r.URL, r.Proto)
			for k, v := range r.Header {
				logger.Printf("%q = %q\n", k, v)
			}
			respBody, _ := ioutil.ReadAll(r.Body)
			logger.Println(string(respBody))
		    logger.Println("===========")
		}
}
*/

func feedbackHandler(w http.ResponseWriter, r *http.Request) {
	logWriter.logger.Println("[STATE] Request Received.")
	logWriter.logger.Printf("Sender: %s\n", r.RemoteAddr)
	if r.Method == "POST" {
		if validateFeedback(r) {
			logWriter.logger.Println("[STATE] Request Validated.")
			logWriter.logger.Println("=========================== FEEDBACK ===========================")
			logWriter.logger.Printf("Method: %s, URL: %s, Proto: %s\n", r.Method, r.URL, r.Proto)
			for k, v := range r.Header {
				logWriter.logger.Printf("%q = %q\n", k, v)
			}
			fb, err := getFeedback(r)
			if err != nil {
				//TODO
			}
			logWriter.logger.Printf("%+v\n", fb)
			logWriter.logger.Println("================================================================")
			req, _ := makeRequest(fb)
			logWriter.logger.Println("[STATE] Issue Created")
			logWriter.logger.Println("=========================== ISSUE ===========================")
			logWriter.logger.Printf("Method: %s, URL: %s, Proto: %s\n", req.Method, req.URL, req.Proto)
			for k, v := range req.Header {
				logWriter.logger.Printf("%q = %q\n", k, v)
			}
			jfb, _ := json.Marshal(fb)
			logWriter.logger.Printf("%s\n", string(jfb))
			logWriter.logger.Println("=============================================================")
			client := &http.Client{}
			client.Do(req)
			logWriter.logger.Println("[STATE] Issue Requested")
		} else {
            logWriter.logger.Println("[STATE] Request Invalidated.")
        }
	} else {
		http.NotFound(w, r)
	}
}

func makeRequest(fb *Feedback) (*http.Request, error) {
	const TOKEN string = "mf-ObB09RoYGsE_GReq3h-q7G3tJUDT0"
	const BTS string = "http://www.feedback.gooroom.kr/api/rest/issues/"
	jfb, _ := json.Marshal(fmt.Sprintf(`{"summary": "%s", "description": "%s", "category": {"name": "%s"}, "project": {"name": "%s"}}`,
		fb.Title, fb.Description, fb.Category, "Gooroom Feedback"))
	req, err := http.NewRequest("POST", BTS, bytes.NewBuffer(jfb))
	defer req.Body.Close()
	req.Header.Add("Authorization", TOKEN)
	req.Header.Add("Content-Type", "application/json")
	return req, err
}

func getFeedback(r *http.Request) (*Feedback, error) {
	var title, category, release, codename, description string
	if err := r.ParseForm(); err != nil {
		log.Print(err)
		return nil, errors.New("Invalid Feedback Request.")
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

func setExpireTimeNow() {
	expireTime.year, expireTime.month, expireTime.day = time.Now().Date()
    expireTime.min = time.Now().Minute() // DELETE
}

func setLogWriterNow() {
    if logWriter.fp != nil {
        if err := logWriter.fp.Close(); err != nil {
            panic(err)
        }
    }

    strv := []string{"gfb-", fmt.Sprintf("%d-%d-%d-%d", expireTime.year, expireTime.month, expireTime.day, expireTime.min), ".log"} // DELETE
    //strv := []string{"gfb-", fmt.Sprintf("%d-%d-%d", expireTime.year, expireTime.month, expireTime.day), ".log"}
    fileName := strings.Join(strv, "")

    fp, err := os.OpenFile(fileName, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0644)
    if err != nil {
        panic(err)
    }

    logWriter.fp = fp
    logWriter.logger = log.New(logWriter.fp, "", log.LstdFlags|log.Lshortfile)
}

func gfbInit() {
    setExpireTimeNow()
    setLogWriterNow()
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
	//http.HandleFunc("/api/rest/issues/", issueHandler) // DELETE

	idleConnsClosed := make(chan struct{})

	go func() {
		done := make(chan os.Signal, 1)
		signal.Notify(done, os.Interrupt, syscall.SIGINT, syscall.SIGTERM)
		<-done

		if err := srv.Shutdown(context.Background()); err != nil {
			logWriter.logger.Printf("HTTP Server Shutdown: %v", err)
		}
		close(idleConnsClosed)
	}()

	if err := srv.ListenAndServe(); err != nil {
		logWriter.logger.Fatalf("HTTP Server ListenAndServe: %v", err)
	}

	<-idleConnsClosed
}
