import sys
from dotenv import load_dotenv
from .Message_maker import Message
from .Serializer import Serializer
import pymysql
import os
load_dotenv()

class DatabaseConnector:
    def __init__(self):
        self.connection = None
    
    def connect(self):
        try:
            self.connection = pymysql.connect(
                host=os.getenv("DB_HOST"),
                database=os.getenv("DB_NAME"),
                user=os.getenv("DB_USERNAME"),
                password=os.getenv("DB_PASSWORD"),
                ssl_ca=os.getenv("SSL_CERT")
            )
            return self.connection
        except:
            return False
    
    def disconnect(self):
        self.connection.close()
    
class DatabaseQueryExecutor:
    def select_keys(conn):
        cursor = conn.cursor(pymysql.cursors.DictCursor)
        
        with conn:
            sql = "select value from api_key"
            cursor.execute(sql)
            result = cursor.fetchall()
        
        return result
        
    def select_seats(conn):
        cursor = conn.cursor(pymysql.cursors.DictCursor)
        
        with conn:
            sql = "select * from seats"
            cursor.execute(sql)
            result = cursor.fetchall()
            result = Serializer.make_serializable_datetimes(result)
        
        return result
    
    def insert_seats(conn, seat_id, user_id, usage_start, usage_end):
        cursor = conn.cursor(pymysql.cursors.DictCursor)
        usage_start = f'"{usage_start}"'
        usage_end = f'"{usage_end}"'
        values = [seat_id, user_id, usage_start, usage_end]
        values = ', '.join(values)
        
        with conn:
            # foreign key constraints check.
            sql = f"select * from users"
            cursor.execute(sql)
            result = cursor.fetchall()
            is_obeying = False
            for record in result:
                if "user_id" in record and record["user_id"] == int(user_id):
                    is_obeying = True
                    break
            
            if not is_obeying:
                raise ValueError("Foreign Key constraint violated.")
            
            sql = f"insert into seats values({values})"
            result = cursor.execute(sql)
            if result:
                conn.commit()
                return True
            else:
                return False
    
    def insert_user(conn, user_id, username):
        cursor = conn.cursor()
        
        with conn.cursor() as cursor:
            sql = f"insert into users values({user_id}, \"{username}\")"
            result = cursor.execute(sql)
            if result:
                conn.commit()
                return Message.Success("Transaction success.")
            else:
                return Message.Success(f"Transaction failed while inserting values {user_id}, \"{username}\"")
    
    def select_users(conn):
        cursor = conn.cursor(pymysql.cursors.DictCursor)
        
        with conn:
            sql = "select * from users"
            cursor.execute(sql)
            result = cursor.fetchall()
        
        return result