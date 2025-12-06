import sys
import time
import requests
import threading
from flask import Flask, request, jsonify

app = Flask(__name__)

# Configuration
GATEWAY_URL = "http://localhost:8080/register"
WORKER_ID = "worker-default"
PORT = 9001

# State
current_load = 0

@app.route('/generate', methods=['POST'])
def generate():
    global current_load
    current_load += 1
    data = request.json
    print(f"[{WORKER_ID}] Received request: {data}")
    
    # Simulate processing
    time.sleep(2)
    
    current_load -= 1
    return jsonify({"status": "success", "worker": WORKER_ID})

@app.route('/health', methods=['GET'])
def health():
    return jsonify({"status": "healthy", "load": current_load})

def register_with_gateway():
    """Registers this worker with the C++ Gateway."""
    payload = {
        "id": WORKER_ID,
        "port": PORT
    }
    try:
        print(f"[{WORKER_ID}] Attempting to register with {GATEWAY_URL}...")
        response = requests.post(GATEWAY_URL, json=payload)
        if response.status_code == 200:
            print(f"[{WORKER_ID}] Successfully registered!")
        else:
            print(f"[{WORKER_ID}] Registration failed: {response.text}")
    except Exception as e:
        print(f"[{WORKER_ID}] Could not connect to gateway: {e}")

if __name__ == '__main__':
    if len(sys.argv) > 1:
        WORKER_ID = sys.argv[1]
    if len(sys.argv) > 2:
        PORT = int(sys.argv[2])

    # Register in a separate thread or just before run (if gateway is up)
    # We'll try once before starting
    register_with_gateway()
    
    print(f"[{WORKER_ID}] Starting worker on port {PORT}")
    app.run(host='0.0.0.0', port=PORT)
