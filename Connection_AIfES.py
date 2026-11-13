"""MNIST test for APP_AIFES, 115200 baud; no TensorFlow dependency.
Run from EAI: python -u Connection_AIfES.py --port /dev/cu.usbmodem1203 --count 100
"""
import argparse,hashlib,json,time
from pathlib import Path
from datetime import datetime,timezone
import numpy as np
import serial

X_SHA='d2c494f9ddff8ffeabed5dbe40b1ac2ed2927a5dcdb667eec7fc6e20282f875b'
Y_SHA='9c34b6666c2e8fd3dd59782bdc64171a8a790681598709ebb83e4047701dbefd'

def read_exact(port,n):
    data=bytearray()
    deadline=time.monotonic()+15
    while len(data)<n and time.monotonic()<deadline:
        data.extend(port.read(n-len(data)))
    if len(data)!=n:
        raise TimeoutError(f'UART: {len(data)}/{n} bytes received; reset board before retrying.')
    return bytes(data)

def infer(port,image):
    port.write(b'\xAB')
    hello=read_exact(port,5)
    if hello[:4]!=b'AIF7':
        raise RuntimeError(f'Wrong firmware/protocol: {hello!r}; expected AIF7.')
    if hello[4]:
        raise RuntimeError(f'AIfES initialization/self-test failed: code {hello[4]}')
    payload=np.asarray(image,dtype='<f4').reshape(784).tobytes()
    if port.write(payload)!=len(payload):raise IOError('Incomplete UART write')
    reply=read_exact(port,11)
    if reply[0]:raise RuntimeError(f'Board frame error: {reply[0]}; reset before retrying.')
    return np.frombuffer(reply[1:],dtype=np.uint8).copy()

def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--port',default='/dev/cu.usbmodem1203')
    ap.add_argument('--count',type=int,default=100)
    ap.add_argument('--x',default='MNIST_xtest_NN_C2_16_10.npy')
    ap.add_argument('--y',default='MNIST_ytest_NN_C2_16_10.npy')
    ap.add_argument('--expected',default='aifes_q7_expected.npy')
    ap.add_argument('--report',default=None)
    args=ap.parse_args()
    if sha(args.x)!=X_SHA or sha(args.y)!=Y_SHA:
        raise ValueError('Dataset differs from the validated NPY files. Check paths before testing.')
    raw=np.load(args.x,allow_pickle=False)
    labels=np.load(args.y,allow_pickle=False).argmax(axis=1)
    if not 1<=args.count<=len(labels):raise ValueError('count must be between 1 and 10000')
    x=np.ascontiguousarray(raw.reshape(-1,784).astype(np.float32)-.5)
    expected=np.load(args.expected,allow_pickle=False)
    if expected.shape!=(len(labels),10) or expected.dtype!=np.int8:
        raise ValueError('Invalid AIfES reference outputs')
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    report_path=Path(args.report or f'test-aifes-{stamp}.json')
    rows=[];correct=0;exact=0
    print(f'AIfES Q7 | {args.port} | {args.count} images | centered inputs')
    with serial.Serial(args.port,115200,timeout=1,write_timeout=10) as port:
        port.reset_input_buffer()
        for i in range(args.count):
            scores=infer(port,x[i])
            pred=int(scores.argmax());truth=int(labels[i])
            match=bool(np.array_equal(scores.astype(np.int16)-128,expected[i].astype(np.int16)))
            correct+=int(pred==truth);exact+=int(match)
            row={'index':i,'expected_class':truth,'prediction':pred,
                 'scores_u8':scores.tolist(),'matches_host_q7':match}
            rows.append(row)
            print(f'{i+1:4d}/{args.count} expected={truth} predicted={pred} '
                  f'accuracy={correct/(i+1):.2%} exact_host={match}',flush=True)
    report={'utc':stamp,'port':args.port,'samples':args.count,'correct':correct,
            'accuracy':correct/args.count,'exact_host_matches':exact,
            'preprocessing':'float32(x_npy) - 0.5','rows':rows}
    report_path.write_text(json.dumps(report,indent=2))
    print(f'FINAL: {correct}/{args.count}; exact Q7 reference: {exact}/{args.count}')
    print(f'Report: {report_path.resolve()}')
    if exact!=args.count:raise SystemExit('Hardware/host Q7 mismatch: inspect before continuing.')

if __name__=='__main__':main()
