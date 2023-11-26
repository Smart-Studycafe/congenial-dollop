from .Database_utils import DatabaseConnector, DatabaseQueryExecutor
import sys

class Certificator:
    def __init__(self):
        # DB로 부터 모든 API KEY 가져옴
        conn = DatabaseConnector().connect()
        if not conn:
            # 연결 실패시 어떻게 하지~
            self.keys = None
        else:
            self.keys = DatabaseQueryExecutor.select_keys(conn)
    
    def get_all_keys(self):
        print(self.keys, file=sys.stdout)
        return self.keys
    
    def check_available(self, api_key):
        print(self.keys, api_key, file=sys.stdout)
        if api_key == "" or self.keys == None:
            return False
        is_containing = any(api_key == item.get("value") for item in self.keys)
        return is_containing