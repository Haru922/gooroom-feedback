package main

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
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
	for ip := range counter.visited {
		delete(counter.visited, ip)
	}
	counter.Unlock()
}

func validateFeedback(r *http.Request) bool {
	updateDate()
	counter.RLock()
	n := counter.visited[r.RemoteAddr]
	counter.RUnlock()
	fmt.Println(r.RemoteAddr, n)
	if n >= 10 {
		return false
	}
	counter.Lock()
	counter.visited[r.RemoteAddr]++
	counter.Unlock()
	return true
}

func updateDate() {
	expireTime.Lock()
	year, month, day := time.Now().Date()
	if expireTime.year != year ||
		expireTime.month != month ||
		expireTime.day != day {
		resetCounter()
		expireTime.year = year
		expireTime.month = month
		expireTime.day = day
	}
	expireTime.Unlock()
}

func issueHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("===issueHandler===")
	fmt.Println("==================")
	fmt.Printf("RemoteAddr: %s\n", r.RemoteAddr)
	if r.Method == "POST" {
		fmt.Printf("%s %s %s\n", r.Method, r.URL, r.Proto)
		for k, v := range r.Header {
			fmt.Printf("Header[%q] = %q\n", k, v)
		}
		respBody, _ := ioutil.ReadAll(r.Body)
		fmt.Printf("Body: %s\n", string(respBody))
	}
	fmt.Println("==================")
}

func feedbackHandler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("===feedbackHandler===")
	fmt.Fprintf(w, "RemoteAddr: %s\n", r.RemoteAddr)
	if r.Method == "POST" {
		if validateFeedback(r) {
			// DEBUG
			fmt.Println("validated.")
			fmt.Fprintf(w, "%s %s %s\n", r.Method, r.URL, r.Proto)
			for k, v := range r.Header {
				fmt.Fprintf(w, "Header[%q] = %q\n", k, v)
			}
			fb, err := getFeedback(r)
			if err != nil {
			}
			req, _ := makeRequest(fb)
			// DEBUG
			for k, v := range req.Header {
				fmt.Fprintf(w, "New Header[%q] = %q\n", k, v)
			}
			jfb, _ := json.Marshal(fb)
			fmt.Fprintf(w, "\njson string: %s\n", string(jfb))
			client := &http.Client{}
			client.Do(req)
            fmt.Println("issue requested")
		}
	} else {
		http.NotFound(w, r)
	}
	fmt.Println("=====================")
}

func makeRequest(fb *Feedback) (*http.Request, error) {
	const TOKEN string = "mf-ObB09RoYGsE_GReq3h-q7G3tJUDT0"
	const BTS string = "http://www.feedback.gooroom.kr/api/rest/issues/"
	jfb, _ := json.Marshal(fmt.Sprintf(`{"summary": "%s", "description": "%s", "category": {"name": "%s"}, "project": {"name": "%s"}}`,
		fb.Title, fb.Description, fb.Category, "Gooroom Feedback"))
	fmt.Println(string(jfb))
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

func main() {
	expireTime.Lock()
	expireTime.year, expireTime.month, expireTime.day = time.Now().Date()
	expireTime.Unlock()

	srv := &http.Server{
		Addr:         "127.0.0.1:8000",
		WriteTimeout: time.Second * 15,
		ReadTimeout:  time.Second * 15,
		IdleTimeout:  time.Second * 60,
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
			log.Printf("HTTP Server Shutdown: %v", err)
		}
		close(idleConnsClosed)
	}()

	if err := srv.ListenAndServe(); err != nil {
		log.Fatalf("HTTP Server ListenAndServe: %v", err)
	}

	<-idleConnsClosed
}
