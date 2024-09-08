#!/usr/bin/env python3
"""a module that handles the log storage in database"""
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker,scope_session, Session, 
from sys import modules


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

    def reload(self) -> None:
        """Reload database engine"""
        Base.metadata.create_all(self.__engine)
        session = sessionmaker(bind=self.__engine, expires_on_commit=False)
        self.__session = scope_session(session)

    def close(self) -> None:
        """close database connection"""
        self.__session.remove()
