# cpp-inference-loadbalancer

a simple load balancer prototype for media specific inference jobs. the idea is, in media inference, cold starts are common, so load balancer prioritizes for minimizing cold starts (prioritizes workers that already have the model/lora loaded in memory). 

this is something that i do for learning more on load balancers and infra, i use the repo and comments as my personal notes. 

i tried to express my thoght process with some multiline comments in the code. make sure to check those out to see how i think about some design decisions.