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
