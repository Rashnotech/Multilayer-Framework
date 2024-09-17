#!/usr/bin/env python3
""" an entry point to the system"""
from flask import Flask, request, jsonify
from flask_cors import CORS
from models import storage
from config.app_config import Config
from routes import app_views
from server.server_hids import start_hids

app = Flask(__name__)

config_name = 'development'
app.config.from_object(Config)
CORS(app, resources={})
app.register_blueprint(app_views)

@app.teardown_appcontext
def teardown_appcontext(self):
    """Tear down econtext"""
    storage.close()

@app.errorhandler(404)
def notfound(error) -> dict:
    """Not found error"""
    return jsonify({'error': 'Not Found!'})

@app.errorhandler(403)
def forbidden(error) -> dict:
    """Forbidden error"""
    return jsonify({'error': 'Forbidden'})



if __name__ == '__main__':
    start_hids()
    app.run(host='0.0.0.0', port=5000)
