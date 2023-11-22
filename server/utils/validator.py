import re

class Validator:
    def validate_user_id(input_id: str):
        pattern = r'^[0-9]+$'
        if re.match(pattern, input_id):
            return True
        else:
            return False
        
    def validate_username(input_name: str):
        pattern = r'^[가-힣a-zA-Z0-9\s]{1,15}$'
        if re.match(pattern, input_name):
            return True  # 일치하는 경우
        else:
            return False  # 불일치하는 경우