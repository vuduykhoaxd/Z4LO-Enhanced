'use strict';

function unsupported() {
    return {
        samples: [],
        errors: [{ pid: 0, op: 'load', code: 'ENOTSUPPORTED' }],
    };
}

function loadNative() {
    if (process.platform === 'win32') {
        if (process.arch === 'x64') {
            return require('./win32_x64/os_sys_info.node');
        }
        if (process.arch === 'ia32') {
            return require('./win32_ia32/os_sys_info.node');
        }
        return null;
    }
    if (process.platform === 'darwin') {
        if (process.arch === 'x64') {
            return require('./darwin_x64/os_sys_info.node');
        }
        if (process.arch === 'arm64') {
            return require('./darwin_arm64/os_sys_info.node');
        }
        return null;
    }
    return null;
}

let native = null;
let loadError = null;
try {
    native = loadNative();
} catch (err) {
    loadError = err;
}

module.exports = {
    isAvailable() {
        return !!native;
    },
    loadError() {
        return loadError;
    },
    async getProcessSnapshot(pids) {
        if (!native) return unsupported();
        const arr = Array.isArray(pids) ? pids : [];
        return native.getProcessSnapshot(arr);
    },
    getOsArchitecture() {
        if (!native) return undefined;
        return native.getOsArchitecture();
    },
    getOsMachine() {
        if (!native) return undefined;
        return native.getOsMachine();
    },
};
