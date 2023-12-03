import re
from datetime import datetime
import sys

class Validator:
    def validate_user_id(input_id: str):
        pattern = r'^[0-9]+$'
        if not re.match(pattern, input_id):
            raise ValueError
        
    def validate_seat_id(input_id: str):
        pattern = r'^[0-9]+$'
        if not re.match(pattern, input_id):
            raise ValueError
        
    def validate_username(input_name: str):
        pattern = r'^[가-힣a-zA-Z0-9\s]{1,15}$'
        if not re.match(pattern, input_name):
            raise ValueError
    
    def validate_user(input_id: str, input_name: str):
        Validator.validate_user_id(input_id)
        Validator.validate_username(input_name)
        return True
    
    def validate_time_range(usage_start: str, usage_end: str):
        start_datetime = datetime.strptime(usage_start, "%Y-%m-%d %H:%M:%S")
        end_datetime = datetime.strptime(usage_end, "%Y-%m-%d %H:%M:%S")
        
        if not end_datetime > start_datetime:
            raise ValueError
    
    def validate_seat(seat_id: str, user_id: str, usage_start: str, usage_end: str):
        # id 확인
        Validator.validate_seat_id(seat_id)
        Validator.validate_user_id(user_id)
        # 날짜 확인
        Validator.validate_time_range(usage_start, usage_end)
        return True
