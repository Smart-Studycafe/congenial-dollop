from .Database_utils import DatabaseConnector, DatabaseQueryExecutor

class Certificator:
    def __init__(self):
        # DB로 부터 모든 API KEY 가져옴
        conn = DatabaseConnector().connect()
        if not conn:
            self.keys = None
        else:
            self.keys = DatabaseQueryExecutor.select_keys(conn)
    
    def get_all_keys(self):
        return self.keys
    
    def check_available(self, api_key):
        if api_key == "" or self.keys == None:
            return False
        is_containing = any(api_key == item.get("value") for item in self.keys)
        return is_containing