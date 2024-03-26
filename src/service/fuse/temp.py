from derecho.cascade.external_client import ServiceClientAPI

capi = ServiceClientAPI()

capi.create_object_pool("/pool1", "VolatileCascadeStoreWithStringKey", 0)
word_to_repeat = "1"
size = 1 # in KB
repetitions = (size * 1024  // len(word_to_repeat)) + 1
mb_string = word_to_repeat * repetitions
encoded_bytes = mb_string.encode('utf-8')
capi.put(f"/pool1/read_test", encoded_bytes)