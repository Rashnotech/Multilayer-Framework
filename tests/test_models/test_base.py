#!/usr/bin/env python3
"""Test on base model"""
import unittest
from models.base import BaseModel


class TestBase(unittest.TestCase):
    """
    A class that best base class
    """

    def test_instantiation(self):
        """Test object is correctly created"""
        base = BaseModel()
        self.assertIs(isinstance(base, 'BaseModel')

