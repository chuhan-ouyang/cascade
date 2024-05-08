from derecho.cascade.external_client import ServiceClientAPI

capi = ServiceClientAPI()

op_pools = [["/pool1", "VolatileCascadeStoreWithStringKey", 0],
            ["/pool2", "PersistentCascadeStoreWithStringKey", 0],
            ["/pool3", "VolatileCascadeStoreWithStringKey", 0]]

for op_pool in op_pools:
    res = capi.create_object_pool(op_pool[0], op_pool[1], op_pool[2])
    if res:
        ver = res.get_result()
    else:
        print(f"Object pool: {op_pool[0]} didn't succesfully create")


objects= [["/pool1/1c", bytes("c", 'utf-8')],
            ["/pool1/2c", bytes("cc", 'utf-8')],
            ["/pool1/3c", bytes("ccc", 'utf-8')],
            ["/pool1/4c", bytes("cccc", 'utf-8')],
            ["/pool1/5c", bytes("ccccc", 'utf-8')],
            ["/pool2/k1", bytes("p2v1", 'utf-8')],
            ["/pool2/k2", bytes("p2v2", 'utf-8')],
            ["/pool3/k1", bytes("p3v1", 'utf-8')],
            ["/pool3/k2", bytes("p3v2", 'utf-8')]]

for object in objects:
    res = capi.put(object[0], object[1])
    if res:
        ver = res.get_result()
    else:
        print(f"object: {object[0]} didn't succesfully put")