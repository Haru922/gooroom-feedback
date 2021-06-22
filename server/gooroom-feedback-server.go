package main

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	//"io/ioutil"
	"log"
	"net/http"
	"os"
	"os/signal"
	"strings"
	"sync"
	"syscall"
	"time"
)

const (
	BACKUP_COUNT   = 5
	MAX_FILE_BYTES = 2 * 1024
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
}{}

var logger *log.Logger

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
		expireTime.day != day {
		resetCounter()
		setExpireTimeNow()
	}
}

func issueHandler(w http.ResponseWriter, r *http.Request) {
	/*
		logger.Printf("RemoteAddr: %s\n", r.RemoteAddr) // DELETE
		if r.Method == "POST" {
		    logger.Println("===ISSUE===") // DELETE
			logger.Printf("%s %s %s\n", r.Method, r.URL, r.Proto) // DELETE
			for k, v := range r.Header { // DELETE
				logger.Printf("%q = %q\n", k, v) // DELETE
			} //DELETE
			respBody, _ := ioutil.ReadAll(r.Body)
			logger.Println(string(respBody)) // DELETE
		    logger.Println("===========") // DELETE
		}
	*/
}

func feedbackHandler(w http.ResponseWriter, r *http.Request) {
	logger.Println("[STATE] Request Received.")
	logger.Printf("Sender: %s\n", r.RemoteAddr)
	if r.Method == "POST" {
		if validateFeedback(r) {
			logger.Println("[STATE] Request Validated.")
			logger.Println("=========================== FEEDBACK ===========================")
			logger.Printf("Method: %s, URL: %s, Proto: %s\n", r.Method, r.URL, r.Proto)
			for k, v := range r.Header {
				logger.Printf("%q = %q\n", k, v)
			}
			fb, err := getFeedback(r)
			if err != nil {
				//TODO
			}
			logger.Println(fb)
			logger.Println("================================================================")
			req, _ := makeRequest(fb)
			logger.Println("[STATE] Issue Created")
			logger.Println("=========================== ISSUE ===========================")
			for k, v := range req.Header {
				logger.Printf("%q = %q\n", k, v)
			}
			jfb, _ := json.Marshal(fb)
			logger.Printf("%s\n", string(jfb))
			logger.Println("=============================================================")
			client := &http.Client{}
			client.Do(req)
			logger.Println("[STATE] Issue Requested")
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
	expireTime.Lock()
	defer expireTime.Unlock()
	expireTime.year, expireTime.month, expireTime.day = time.Now().Date()
}

func main() {
	setExpireTimeNow()

	fp, err := os.OpenFile("gfb.log", os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0644)
	if err != nil {
		panic(err)
	}
	defer fp.Close()

	logger = log.New(fp, "", log.LstdFlags|log.Lshortfile)

	srv := &http.Server{
		Addr:         ":8000",
		WriteTimeout: 15 * time.Second,
		ReadTimeout:  15 * time.Second,
		IdleTimeout:  60 * time.Second,
		Handler:      nil,
	}
	http.HandleFunc("/gooroom/feedback/new", feedbackHandler)
	http.HandleFunc("/api/rest/issues/", issueHandler)

	idleConnsClosed := make(chan struct{})

	go func() {
		done := make(chan os.Signal, 1)
		signal.Notify(done, os.Interrupt, syscall.SIGINT, syscall.SIGTERM)
		<-done

		if err := srv.Shutdown(context.Background()); err != nil {
			logger.Printf("HTTP Server Shutdown: %v", err)
		}
		close(idleConnsClosed)
	}()

	if err := srv.ListenAndServe(); err != nil {
		logger.Fatalf("HTTP Server ListenAndServe: %v", err)
	}

	<-idleConnsClosed
}
