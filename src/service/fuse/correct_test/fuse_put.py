from derecho.cascade.external_client import ServiceClientAPI

capi = ServiceClientAPI()

# Create object pool
capi.create_object_pool("/pool1", "VolatileCascadeStoreWithStringKey", 0)
capi.create_object_pool("/pool2", "VolatileCascadeStoreWithStringKey", 0)
capi.create_object_pool("/pool3", "VolatileCascadeStoreWithStringKey", 0)

# Put in object pool
capi.put("/pool1/k1", bytes("p1v1", 'utf-8'))
capi.put("/pool1/k2", bytes("p1v2", 'utf-8'))
capi.put("/pool2/k1", bytes("p2v1", 'utf-8'))
capi.put("/pool2/k2", bytes("p2v2", 'utf-8'))
capi.put("/pool3/k1", bytes("p3v1", 'utf-8'))
capi.put("/pool3/k2", bytes("p3v2", 'utf-8'))
