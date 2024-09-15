#!/usr/bin/python3
"""Blueprint for the api"""
from flask import Blueprint, abort, jsonify


app_views = Blueprint('app_views', __name__, url_prefix='/api/v1')


from .reports import *
#from .routes.devices import *
#from .routes.network import *
