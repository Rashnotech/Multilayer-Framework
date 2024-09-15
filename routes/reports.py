#!/usr/bin/env python3
"""a module for report on IDS"""
from flask import jsonify, abort, request
from . import app_views


@app_views.route('/report', methods=['GET'], strict_slashes=False)
def report():
    return jsonify({'message': 'Successful'})
