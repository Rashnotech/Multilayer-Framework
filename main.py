#!/usr/bin/env python3
""" an entry point to the system"""
from flask import Flask, request, jsonify


app = Flask(__name__)

@app.handler


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
