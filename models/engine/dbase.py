#!/usr/bin/env python3
"""a module that handles the log storage in database"""
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker, scoped_session, Session
from sys import modules
from models.base import Base
from models.black_list import BlackList


class DB:
    """
    A class that represent Database engine 
    """
    __engine = None
    __session = None

    def __init__(self, name) -> None:
        """intialization method"""
        self.__engine = create_engine(f'sqlite:///{name}')
        self.reload()
    
    def all(self, cls=None):
        """
            Query all object in the current database session.
            Args:
                cls (class): The class to query.
            Return:
                dict: A dictionary with keys in this format
                <class-name>.<object-id>
        """
        obj_dict = {}
        if cls:
            cls = getattr(modules[__name__], cls.__name__)
            result = self.__session.query(cls).all()
        else:
            result = []
            for class_name in classes:
                result.extend(self.__session.query(classes[class_name]).all())
        for obj in result:
            key = '{}.{}'.format(type(obj).__name__, obj.id)
            obj_dict[key] = obj
        return obj_dict

    def new(self, obj):
        """add a new object to the database"""
        self.__session.add(obj)

    def rollback(self):
        """Rollback all changes of the current database session"""
        self.__session.rollback()

    def save(self):
        """save an object to the database"""
        try:
            self.__session.commit()
        except:
            self.rollback()
            raise
     
    def delete(self, obj=None):
        """delete an object from the database"""
        if obj:
            self.__session.delete(obj)

    def get(self, cls, id):
        """Retrieve objects from storage"""
        obj = self.__session.query(cls).filter_by(id=id).first()
        return obj

    def reload(self) -> None:
        """Reload database engine"""
        Base.metadata.create_all(self.__engine)
        session = sessionmaker(bind=self.__engine, expires_on_commit=False)
        self.__session = scoped_session(session)

    def close(self) -> None:
        """close database connection"""
        self.__session.remove()
