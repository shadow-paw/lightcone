# Example: Simple http server
This example demostrate how easy to develop an REST server.


# Stress test result with k6.io
Machine: Intel 8700k  
OS: Ubuntu 18.04 LTS  
Test for: `GET /`  
```
          /\      |‾‾|  /‾‾/  /‾/   
     /\  /  \     |  |_/  /  / /    
    /  \/    \    |      |  /  ‾‾\  
   /          \   |  |‾\  \ | (_) | 
  / __________ \  |__|  \__\ \___/ .io

  execution: local
     output: -
     script: /stress/stress.js

    duration: 30s, iterations: -
         vus: 100, max: 100

    done [==========================================================] 30s / 30s

    data_received..............: 127 MB  4.2 MB/s
    data_sent..................: 226 MB  7.5 MB/s
    http_req_blocked...........: avg=1.48µs   min=680ns   med=1.14µs   max=24.15ms p(90)=1.45µs p(95)=1.64µs 
    http_req_connecting........: avg=119ns    min=0s      med=0s       max=23.93ms p(90)=0s     p(95)=0s     
    http_req_duration..........: avg=632.68µs min=39.29µs med=173.5µs  max=24.31ms p(90)=1.89ms p(95)=2.92ms 
    http_req_receiving.........: avg=9.59µs   min=3.23µs  med=7.27µs   max=23.02ms p(90)=9.83µs p(95)=13.73µs
    http_req_sending...........: avg=8.19µs   min=3.54µs  med=5.95µs   max=23.22ms p(90)=9.91µs p(95)=14.04µs
    http_req_tls_handshaking...: avg=0s       min=0s      med=0s       max=0s      p(90)=0s     p(95)=0s     
    http_req_waiting...........: avg=614.89µs min=28.15µs med=159.01µs max=24.26ms p(90)=1.87ms p(95)=2.9ms  
    http_reqs..................: 2827607 94253.005582/s
    iteration_duration.........: avg=680.36µs min=62.67µs med=211.96µs max=25.03ms p(90)=1.96ms p(95)=3ms    
    iterations.................: 2827554 94251.238926/s
    vus........................: 100     min=100 max=100
    vus_max....................: 100     min=100 max=100
```

Same test on [nodejs reference implementation](nodejs/server.js)  
Run with `docker run --rm -it $(docker build -q .)`  
```
          /\      |‾‾|  /‾‾/  /‾/   
     /\  /  \     |  |_/  /  / /    
    /  \/    \    |      |  /  ‾‾\  
   /          \   |  |‾\  \ | (_) | 
  / __________ \  |__|  \__\ \___/ .io

  execution: local
     output: -
     script: /stress/stress.js

    duration: 30s, iterations: -
         vus: 100, max: 100

    done [==========================================================] 30s / 30s

    data_received..............: 251 MB  8.4 MB/s
    data_sent..................: 98 MB   3.3 MB/s
    http_req_blocked...........: avg=1.96µs min=691ns   med=1.34µs max=36.97ms p(90)=1.75µs  p(95)=1.91µs 
    http_req_connecting........: avg=389ns  min=0s      med=0s     max=36.93ms p(90)=0s      p(95)=0s     
    http_req_duration..........: avg=1.86ms min=98.57µs med=1.66ms max=50.29ms p(90)=2.99ms  p(95)=3.75ms 
    http_req_receiving.........: avg=12.3µs min=3.57µs  med=8.02µs max=15.26ms p(90)=13.17µs p(95)=16.53µs
    http_req_sending...........: avg=9.04µs min=3.8µs   med=6.81µs max=37.58ms p(90)=9.39µs  p(95)=11.71µs
    http_req_tls_handshaking...: avg=0s     min=0s      med=0s     max=0s      p(90)=0s      p(95)=0s     
    http_req_waiting...........: avg=1.84ms min=84.88µs med=1.64ms max=50.14ms p(90)=2.96ms  p(95)=3.72ms 
    http_reqs..................: 1224749 40824.861892/s
    iteration_duration.........: avg=1.92ms min=136.5µs med=1.71ms max=87.2ms  p(90)=3.06ms  p(95)=3.84ms 
    iterations.................: 1224681 40822.595232/s
    vus........................: 100     min=100 max=100
    vus_max....................: 100     min=100 max=100
```
